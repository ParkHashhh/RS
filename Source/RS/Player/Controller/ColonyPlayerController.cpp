// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/Controller/ColonyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "RS/Core/LogCategory.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"
#include "RS/GameFramework/GameMode/ColonyGameMode.h"
#include "RS/GameFramework/RSGameInstance.h"
#include "RS/GameFramework/GameMode/RSInGameMode.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/UI/Colony/Result/StageResultWidget.h"

AColonyPlayerController::AColonyPlayerController()
{
	bShowMouseCursor = true;
}

void AColonyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| ColonyPC BeginPlay, bShowMouseCursor: %d"), *LOG_LINE_INFO,
		bShowMouseCursor);
	RS_PRINT_SCREEN(15.f, FColor::Green, TEXT("[ColonyPC] BeginPlay, Map: %s"), *GetWorld()->GetMapName());

	if (IsValid(ColonyWidgetClass))
	{
		ColonyWidget = CreateWidget<UUserWidget>(GetWorld(), ColonyWidgetClass);
		if (IsValid(ColonyWidget))
		{
			ColonyWidget->AddToViewport();

			if (!IsValid(ActiveStageResultWidget) || !ActiveStageResultWidget->IsInViewport())
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(ColonyWidget->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				SetInputMode(InputMode);
			}
		}
	}

	if (IsValid(MapSelectWidgetClass))
	{
		MapSelectWidget = CreateWidget<UUserWidget>(GetWorld(), MapSelectWidgetClass);
	}

	if (auto* GI = GetGameInstance<URSGameInstance>())
	{
		GI->StopBGM();

		if (const auto* Storage = GI->GetSubsystem<UDataStorageSubsystem>())
		{
			RS_LOG_DEBUG(LogRSSession, Warning,
				TEXT("%s| Sending Server_SetSelectedCharacterInfo, Texture: %s, Skill: %s"),
				*LOG_LINE_INFO, *Storage->GetSelectedTextureName().ToString(), *Storage->GetSelectedSkillName().ToString());
			Server_SetSelectedCharacterInfo(Storage->GetSelectedTextureName(), Storage->GetSelectedSkillName());
		}
	}
}

void AColonyPlayerController::ApplyGameplayInputMode()
{
	if (IsValid(ActiveStageResultWidget) && ActiveStageResultWidget->IsInViewport())
	{
		RS_LOG_DEBUG(LogRSSession, Warning,
			TEXT("%s| ApplyGameplayInputMode SKIPPED, ActiveStageResultWidget is in viewport"), *LOG_LINE_INFO);
		return;
	}

	Super::ApplyGameplayInputMode();
}

void AColonyPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s| OnPossess %s"), *LOG_LINE_INFO, *GetNameSafe(aPawn));
	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| OnPossess %s"), *LOG_LINE_INFO, *GetNameSafe(aPawn));
}

void AColonyPlayerController::Server_SetSelectedCharacterInfo_Implementation(FName TextureName, FName SkillName)
{
	RS_LOG_DEBUG(LogRSSession, Warning,
		TEXT("%s| Server_SetSelectedCharacterInfo_Implementation CALLED, Texture: %s, Skill: %s, Controller: %s"),
		*LOG_LINE_INFO, *TextureName.ToString(), *SkillName.ToString(), *GetName());

	if (auto* GM = GetWorld()->GetAuthGameMode<AColonyGameMode>())
	{
		GM->OnPlayerCharacterSelected(this, TextureName, SkillName);
	}
	else
	{
		RS_LOG_DEBUG(LogRSSession, Error, TEXT("%s| GameMode cast to AColonyGameMode failed"), *LOG_LINE_INFO);
	}
}

void AColonyPlayerController::ShowMapSelectWidget()
{
	if (IsValid(MapSelectWidget))
	{
		if (MapSelectWidget->IsInViewport())
		{
			RemoveMapSelectWidget();
			return;
		}
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
		MapSelectWidget->RemoveFromParent();
		UGameplayStatics::PlaySound2D(this, MapSelectCloseSound);

		if (IsValid(ActiveStageResultWidget) && ActiveStageResultWidget->IsInViewport())
		{
			bShowMouseCursor = true;

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(ActiveStageResultWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
		}
		else
		{
			bShowMouseCursor = false;

			FInputModeGameOnly InputMode;
			SetInputMode(InputMode);
		}
	}
}

void AColonyPlayerController::ShowStageResultWidget()
{
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| ShowStageResultWidget CALLED, bShowMouseCursor before: %d"),
		*LOG_LINE_INFO, bShowMouseCursor);
	if (!IsValid(StageResultWidgetClass))
	{
		RS_PRINT_SCREEN(100.f, FColor::Red, TEXT("ShowStageResultWidget: StageResultWidgetClass is NOT set"));
		return;
	}

	auto* Widget = CreateWidget<UStageResultWidget>(this, StageResultWidgetClass);
	if (!IsValid(Widget))
	{
		return;
	}

	ActiveStageResultWidget = Widget;
	Widget->AddToViewport();

	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AColonyPlayerController::Server_RequestPortal_Implementation(FName LevelName, const FString& PackagePath)
{
	ARSInGameMode* GM = GetWorld()->GetAuthGameMode<ARSInGameMode>();
	if (IsValid(GM))
	{
		GM->CreateOrUpdatePortal(LevelName, PackagePath);
	}
}

void AColonyPlayerController::Client_ShowStageResultWidget_Implementation(int32 InKillCount, float InPlayTime, const TArray<FItemStorageInfo>& InNewItems)
{
	PendingStageResultKillCount = InKillCount;
	PendingStageResultPlayTime = InPlayTime;
	PendingStageResultNewItems = InNewItems;
	ShowStageResultWidget();
}

void AColonyPlayerController::Server_ClearPendingStageResult_Implementation()
{
	if (const ARSPlayerState* PS = GetPlayerState<ARSPlayerState>())
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				StorageSubsystem->ConsumePendingStageResult(PS->GetUniqueId());
			}
		}
	}
}
