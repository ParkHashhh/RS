// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/RSUtils.h"

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

		MapSelectWidget->RemoveFromParent();
	}
}

void AColonyPlayerController::Server_EnterStage_Implementation(FName LevelName, const FString& PackagePath)
{
	if (URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance()))
	{
		GI->LevelLoading(LevelName, PackagePath, TEXT(""), false, true);
	}
}
