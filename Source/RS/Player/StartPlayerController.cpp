// Fill out your copyright notice in the Description page of Project Settings.


#include "StartPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "RS/LogCategory.h"
#include "RS/UI/Start/SettingWidget.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/GameMode/RSGameInstance.h"

AStartPlayerController::AStartPlayerController()
{
	bShowMouseCursor = true;
}

bool AStartPlayerController::TryStartSinglePlayer()
{
	if (auto* GI = Cast<URSGameInstance>(GetGameInstance()))
	{
		GI->SetSinglePlayerMode(true);
	}

	if (auto* PS = GetPlayerState<ARSPlayerState>())
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				return StorageSubsystem->LoadPlayerDataFromDisk(PS);
			}
		}
	}

	return false;
}

void AStartPlayerController::StartMultiPlayer()
{
	if (auto* GI = Cast<URSGameInstance>(GetGameInstance()))
	{
		GI->SetSinglePlayerMode(false);
	}
}

void AStartPlayerController::BeginPlay()
{
	RS_PRINT_SCREEN(15.f, FColor::Cyan, TEXT("[StartPC] BeginPlay, Map: %s"), *GetWorld()->GetMapName());
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
