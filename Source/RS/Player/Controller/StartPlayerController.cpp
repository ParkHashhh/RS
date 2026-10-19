// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/Controller/StartPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "RS/Core/LogCategory.h"
#include "RS/UI/Start/SettingWidget.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"
#include "RS/GameFramework/RSGameInstance.h"

AStartPlayerController::AStartPlayerController()
{
	bShowMouseCursor = true;
}

bool AStartPlayerController::TryStartSinglePlayer()
{
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| TryStartSinglePlayer CALLED"), *LOG_LINE_INFO);

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
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| StartMultiPlayer CALLED"), *LOG_LINE_INFO);

	if (auto* GI = Cast<URSGameInstance>(GetGameInstance()))
	{
		GI->SetSinglePlayerMode(false);
	}

	// DataStorageSubsystem은 GameInstance 수명 동안 유지되는 값이라, 같은 실행 중에 싱글플레이를
	// 하고 왔으면 그때의 난이도 스테이지/인벤토리 스냅샷이 남아있음 - 새 멀티 세션은 항상 깨끗하게 시작해야 함
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
		{
			StorageSubsystem->SetCurrentDifficultyStage(1);

			if (auto* PS = GetPlayerState<ARSPlayerState>())
			{
				StorageSubsystem->ClearPlayerInventory(PS->GetUniqueId());
			}
		}
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
	if (auto* GI = Cast<URSGameInstance>(GetGameInstance()))
	{
		GI->PlayBGM();
	}
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
