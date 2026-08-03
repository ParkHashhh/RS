// Fill out your copyright notice in the Description page of Project Settings.


#include "StartPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "RS/LogCategory.h"
#include "RS/UI/Start/SettingWidget.h"

AStartPlayerController::AStartPlayerController()
{
	bShowMouseCursor = true;
}

void AStartPlayerController::BeginPlay()
{
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);


	if (IsValid(StartWidgetClass))
	{
		StartWidget = CreateWidget<UUserWidget>(GetWorld(), StartWidgetClass);
		if (IsValid(StartWidget))
		{
			CurrentMouseCursor = EMouseCursor::Custom;
			StartWidget->AddToViewport();
		}
	}
	CreateSettingWidget();
}

void AStartPlayerController::CreateSettingWidget()
{
	if (!IsValid(SettingWidgetClass))
	{
		return;
	}

	auto* SettingWidget = CreateWidget<USettingWidget>(this, SettingWidgetClass);
	if (!IsValid(SettingWidget))
	{
		return;
	}

	SettingWidget->AddToViewport();
	SettingWidget->SetVisibility(ESlateVisibility::Collapsed);
}
