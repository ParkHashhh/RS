// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/GameMode/RSInGameMode.h"
#include "RS/RSUtils.h"
#include "Kismet/GameplayStatics.h"
AColonyPlayerController::AColonyPlayerController()
{
	bShowMouseCursor = true;
}


void AColonyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ColonyWidgetClass))
	{
		ColonyWidget = CreateWidget<UUserWidget>(GetWorld(), ColonyWidgetClass);
		if (IsValid(ColonyWidget))
		{
			ColonyWidget->AddToViewport();

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(ColonyWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
		}
	}
	if (IsValid(MapSelectWidgetClass))
	{
		MapSelectWidget = CreateWidget<UUserWidget>(GetWorld(), MapSelectWidgetClass);
	}

	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	if (IsValid(GI))
	{
		GI->StopBGM();
	}
}

void AColonyPlayerController::ShowMapSelcetWidget()
{
	if (IsValid(MapSelectWidget))
	{
		bShowMouseCursor = true;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MapSelectWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);		
		UGameplayStatics::PlaySound2D(this, MapSelectOpenSound);
		MapSelectWidget->AddToViewport();
	}
}

void AColonyPlayerController::RemoveMapSelectWidget()
{
	if (IsValid(MapSelectWidget))
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		UGameplayStatics::PlaySound2D(this, MapSelectCloseSound);
		MapSelectWidget->RemoveFromParent();
	}
}

void AColonyPlayerController::Server_RequestPortal_Implementation(FName LevelName, const FString& PackagePath)
{
	ARSInGameMode* GM = GetWorld()->GetAuthGameMode<ARSInGameMode>();
	if (IsValid(GM))
	{
		GM->CreateOrUpdatePortal(LevelName, PackagePath);
	}
}
