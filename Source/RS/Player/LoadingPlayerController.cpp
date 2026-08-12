// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingPlayerController.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/UI/LoadingUserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "RS/LogCategory.h"
#include "ShaderCompiler.h"

namespace
{
	void AddTravelOptionTokens(const FString& Options, TArray<FString>& OutOptions)
	{
		const FString TrimmedOptions = Options.TrimStartAndEnd();
		if (TrimmedOptions.IsEmpty())
		{
			return;
		}

		TArray<FString> SpaceSeparatedOptions;
		TrimmedOptions.ParseIntoArrayWS(SpaceSeparatedOptions);
		for (const FString& SpaceSeparatedOption : SpaceSeparatedOptions)
		{
			TArray<FString> QuestionSeparatedOptions;
			SpaceSeparatedOption.ParseIntoArray(QuestionSeparatedOptions, TEXT("?"), true);
			for (const FString& Option : QuestionSeparatedOptions)
			{
				const FString TrimmedOption = Option.TrimStartAndEnd();
				if (!TrimmedOption.IsEmpty())
				{
					OutOptions.Add(TrimmedOption);
				}
			}
		}
	}

	bool ContainsTravelOption(const TArray<FString>& Options, const FString& OptionName)
	{
		for (const FString& Option : Options)
		{
			if (Option.Equals(OptionName, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}

		return false;
	}

	FString JoinTravelOptions(const TArray<FString>& Options)
	{
		return FString::Join(Options, TEXT("?"));
	}

	FString NormalizeTravelOptions(const FString& Options)
	{
		TArray<FString> OptionTokens;
		AddTravelOptionTokens(Options, OptionTokens);
		return JoinTravelOptions(OptionTokens);
	}

	FString BuildOpenLevelOptions(const FString& Options, bool bListen)
	{
		TArray<FString> OptionTokens;
		AddTravelOptionTokens(Options, OptionTokens);
		if (bListen && !ContainsTravelOption(OptionTokens, TEXT("listen")))
		{
			OptionTokens.Insert(TEXT("listen"), 0);
		}

		return JoinTravelOptions(OptionTokens);
	}

	FString BuildTravelURL(const FString& BaseURL, const FString& Options)
	{
		const FString TrimmedBaseURL = BaseURL.TrimStartAndEnd();
		const FString NormalizedOptions = NormalizeTravelOptions(Options);
		if (NormalizedOptions.IsEmpty())
		{
			return TrimmedBaseURL;
		}

		return TrimmedBaseURL.EndsWith(TEXT("?"))
			       ? TrimmedBaseURL + NormalizedOptions
			       : TrimmedBaseURL + TEXT("?") + NormalizedOptions;
	}
}

ALoadingPlayerController::ALoadingPlayerController()
{
	bShowMouseCursor = false;
	PrimaryActorTick.bCanEverTick = true;
}

void ALoadingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(LoadingWidgetClass))
	{
		LoadingWidget = CreateWidget<ULoadingUserWidget>(GetWorld(), LoadingWidgetClass);
		if (IsValid(LoadingWidget))
		{
			LoadingWidget->AddToViewport();

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(LoadingWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			SetInputMode(InputMode);
		}
	}

	if (const auto* GI = GetGameInstance<URSGameInstance>())
	{
		NextLevelName = GI->GetNextLevelName();
		LoadingPackagePath = GI->GetNextLevelPackagePath();
		LoadingStartTime = GetWorld()->GetTimeSeconds();
		bNextLevelIsListen = GI->IsListen();
		NextLevelOptions = GI->GetNextLevelOptions();
		bIsServerTravel = GI->IsServerTravel();

		StartAsyncLoading();
	}
}

void ALoadingPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bHasTraveled)
	{
		return;
	}

	// 1단계: 패키지 비동기 로딩 진행률 (0 ~ 80%)
	if (!bLoadingPackage)
	{
		float PackagePercent = GetAsyncLoadPercentage(FName(*LoadingPackagePath));
		if (PackagePercent >= 0.f && IsValid(LoadingWidget))
		{
			LoadingWidget->SetLoadingProgress((PackagePercent / 100.f) * 0.8f);
		}
		return;
	}

	// 2단계: 쉐이더 컴파일 대기 (80% 고정, 다 끝나야 다음으로)
	if (GShaderCompilingManager && GShaderCompilingManager->IsCompiling())
	{
		if (IsValid(LoadingWidget))
		{
			LoadingWidget->SetLoadingProgress(0.8f);
		}
		return;
	}

	// 3단계: 완료, 실제 이동
	if (IsValid(LoadingWidget))
	{
		LoadingWidget->SetLoadingProgress(1.f);
	}

	CompleteLoadingAndTravel();
}

void ALoadingPlayerController::StartAsyncLoading()
{
	if (bLoadingStart)
	{
		return;
	}

	bLoadingStart = true;

	LoadPackageAsync(LoadingPackagePath, FLoadPackageAsyncDelegate::CreateUObject(this,
		                 &ALoadingPlayerController::OnPackageLoading), 0, EPackageFlags::PKG_ContainsMapData);
}

void ALoadingPlayerController::OnPackageLoading(const FName& PackageName, UPackage* LoadPackage,
                                                EAsyncLoadingResult::Type Result)
{
	if (Result == EAsyncLoadingResult::Succeeded)
	{
		LoadingPackageCompleteName = PackageName;
		bLoadingPackage = true;
		LoadingPackage = LoadPackage;

		if (LoadingPackage)
		{
			LoadingPackage->AddToRoot();
		}
	}
	else
	{
		RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("MapLoading Failed : %s"), *LoadingPackagePath);
	}
}

void ALoadingPlayerController::CompleteLoadingAndTravel()
{
	if (bHasTraveled)
	{
		return;
	}

	bHasTraveled = true;

	if (URSGameInstance* GI = GetGameInstance<URSGameInstance>())
	{
		RS_LOG_DEBUG(LogRSSession, Warning,
		             TEXT(
			             "%s| CompleteLoadingAndTravel: NextLevelName=%s LoadingPackagePath=%s NextLevelOptions=%s bIsServerTravel=%s bNextLevelIsListen=%s"
		             ),
		             *LOG_LINE_INFO,
		             *NextLevelName.ToString(),
		             *LoadingPackagePath,
		             *NextLevelOptions,
		             bIsServerTravel ? TEXT("true") : TEXT("false"),
		             bNextLevelIsListen ? TEXT("true") : TEXT("false"));

		FString ClientConnectString;
		FString ClientTravelOptions;

		const bool bConsumedPendingClientTravel = GI->ConsumePendingClientTravel(
			ClientConnectString, ClientTravelOptions);

		RS_LOG_DEBUG(LogRSSession, Warning,
		             TEXT("%s| ConsumePendingClientTravel: Result=%s ConnectString=%s Options=%s"),
		             *LOG_LINE_INFO,
		             bConsumedPendingClientTravel ? TEXT("true") : TEXT("false"),
		             *ClientConnectString,
		             *ClientTravelOptions);

		if (bConsumedPendingClientTravel)
		{
			const FString CombinedURL = ClientConnectString + TEXT("/") + NextLevelName.ToString();
			const FString FinalClientTravelURL = BuildTravelURL(CombinedURL, ClientTravelOptions);

			RS_LOG_DEBUG(LogRSSession, Warning,
			             TEXT("%s| ClientTravel URL: %s"),
			             *LOG_LINE_INFO,
			             *FinalClientTravelURL);

			ClientTravel(FinalClientTravelURL, TRAVEL_Absolute);
		}
		else if (bIsServerTravel)
		{
			if (HasAuthority())
			{
				GetWorld()->ServerTravel(BuildTravelURL(NextLevelName.ToString(), NextLevelOptions));
			}
		}
		else
		{
			UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true,
			                            BuildOpenLevelOptions(NextLevelOptions, bNextLevelIsListen));
		}
	}
	else if (bIsServerTravel)
	{
		if (HasAuthority())
		{
			GetWorld()->ServerTravel(BuildTravelURL(NextLevelName.ToString(), NextLevelOptions));
		}
	}
	else
	{
		UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true,
		                            BuildOpenLevelOptions(NextLevelOptions, bNextLevelIsListen));
	}

	RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("MapLoading Succeeded : %s"), *LoadingPackagePath);

	if (LoadingPackage)
	{
		LoadingPackage->RemoveFromRoot();
		LoadingPackage = nullptr;
	}
}
