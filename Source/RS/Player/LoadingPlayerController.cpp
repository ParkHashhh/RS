// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingPlayerController.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/UI/LoadingUserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "RS/LogCategory.h"
#include "ShaderCompiler.h"

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

	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	if (GI)
	{
		NextLevelName = GI->GetNextLevelName();
		LoadingPackagePath = GI->GetNextLevelPackagePath();
		LoadingStartTime = GetWorld()->GetTimeSeconds();
		bNextLevelIsListen = GI->GetIsListen();
		NextLevelOptions = GI->GetNextLevelOptions();
		bIsServerTravel = GI->GetIsServerTravel();
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
		return;

	bLoadingStart = true;
	 

	LoadPackageAsync(LoadingPackagePath, FLoadPackageAsyncDelegate::CreateUObject(this,
		&ALoadingPlayerController::OnPackageLoading), 0, EPackageFlags::PKG_ContainsMapData);
}

void ALoadingPlayerController::OnPackageLoading(const FName& PackageName,
	UPackage* LoadPackage, EAsyncLoadingResult::Type Result)
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

	FString FinalOptions = NextLevelOptions;
	int32 RandomPort = FMath::RandRange(7000, 7999);
	if (bNextLevelIsListen)
	{
		FinalOptions = FString::Printf(TEXT("listen Port=%d "), RandomPort) + NextLevelOptions;
	}

	if (bIsServerTravel)
	{
		if (HasAuthority())
		{
			GetWorld()->ServerTravel(NextLevelName.ToString() + FinalOptions);
		}
	}

	else
	{
		UGameplayStatics::OpenLevel(GetWorld(), NextLevelName, true, FinalOptions);
	}

	RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("MapLoading Succeeded : %s"), *LoadingPackagePath);
	if (LoadingPackage)
	{
		LoadingPackage->RemoveFromRoot();
		LoadingPackage = nullptr;
	}
}
