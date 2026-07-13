// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingPlayerController.h"


#include "Blueprint/UserWidget.h"
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

}

