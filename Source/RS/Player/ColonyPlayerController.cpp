// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "RS/LogCategory.h"
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
}


void AColonyPlayerController::Server_EnterStage_Implementation()
{
	RSUtils::ServerTravelWithInventory(GetWorld(), TEXT("Stage_Warehouse"), true);
}
