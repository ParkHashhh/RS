// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingPlayerController.h"
#include "RS/GameMode/RSGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "RS/LogCategory.h"

ALoadingPlayerController::ALoadingPlayerController()
{
	bShowMouseCursor = false;
}

void ALoadingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(LoadingWidgetClass))
	{

		LoadingWidget = CreateWidget<UUserWidget>(GetWorld(), LoadingWidgetClass);
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
		FString FinalOptions = NextLevelOptions;
		int32 RandomPort = FMath::RandRange(7000, 7999);
		if (bNextLevelIsListen)
		{
			FinalOptions = FString::Printf(TEXT("listen Port=%d "), RandomPort) + NextLevelOptions;
			RS_PRINT_SCREEN(100.f, FColor::Green, TEXT("PortNumber :  %d"), RandomPort);
			RS_PRINT_SCREEN(100.f, FColor::Red, TEXT("Final Options : %s"), *FinalOptions);

		}
		if (bIsServerTravel)
		{
			if (HasAuthority())
			{
				UE_LOG(LogTemp, Warning, TEXT("Calling ServerTravel!"));
				GetWorld()->ServerTravel(NextLevelName.ToString() + FinalOptions);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("HasAuthority FALSE - ServerTravel skipped!"));
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
	else
	{
		RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("MapLoading Failed : %s"), *LoadingPackagePath);
	}
}

