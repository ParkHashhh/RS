// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "PlayerCharacterBase.h"
#include "RSPlayerState.h"
#include "RS/LogCategory.h"
#include "RS/Interaction/InteractableComponent.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/UI/Battle/BattleHUDWidget.h"
#include "RS/UI/Battle/Reward/RewardPopupWidget.h"
#include "RS/UI/PauseWidget.h"
#include "RS/UI/Start/SettingWidget.h"
#include "RS/GameMode/RSGameState.h"
#include "RS/UI/RSGameUserSettings.h"

APlayerControllerBase::APlayerControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		CurrentMouseCursor = EMouseCursor::Default;
		// 입력모드를 세팅한다
		// FInputModeGameOnly
		// FInputModeGameAndUI
		// FInputModeUIOnly

		CurrentMouseCursor = EMouseCursor::Custom;

		FInputModeGameAndUI InputMode;
		SetInputMode(InputMode);

		CreateBattleHUD();
		CreateEndPopUp();
		CreatePauseWidget();
		CreateSettingWidget();
		CreateControlHelpWidget();
	}
}

void APlayerControllerBase::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
}

void APlayerControllerBase::OnUnPossess()
{
	Super::OnUnPossess();
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerControllerBase::ApplyGameplayInputMode()
{
	FInputModeGameOnly InputModeData;
	InputModeData.SetConsumeCaptureMouseDown(true);
	SetInputMode(InputModeData);
	bShowMouseCursor = false;

	if (UGameViewportClient* VC = GetWorld() ? GetWorld()->GetGameViewport() : nullptr)
	{
		if (FViewport* Viewport = VC->Viewport)
		{
			Viewport->CaptureMouse(true);
			Viewport->LockMouseToViewport(true);
		}
	}
}

void APlayerControllerBase::CreateBattleHUD()
{
	if (!IsValid(BattleHUDWidgetClass))
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: BattleHUDWidgetClass is not valid"), *LOG_LINE_INFO);
		return;
	}

	auto* BattleHUDWidget = CreateWidget<UBattleHUDWidget>(this, BattleHUDWidgetClass);
	if (!IsValid(BattleHUDWidget))
	{
		RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: BattleHUDWidget is not valid"), *LOG_LINE_INFO);
		return;
	}

	RS_LOG_DEBUG(LogRSInit, Log, TEXT("%s: BattleHUDWidget is created"), *LOG_LINE_INFO);
	RS_PRINT_SCREEN(5.f, FColor::Black, TEXT("HUD Create"));

	BattleHUDWidget->AddToViewport();

	BackupBattleHUDWidget = BattleHUDWidget;

	//HUD 위젯 생성이 보스좀비 포인터 등록보다 늦으므로 이곳에서 보스 체력바 위젯을 제어해야 한다.
	if (GetWorld())
	{
		ARSGameState* GameState = GetWorld()->GetGameState<ARSGameState>();
		if (GameState)
		{
			if (GameState->GetCurrentBoss() != nullptr)
			{
				GameState->ChangeBossHPBarActivation();
				RS_PRINT_SCREEN(5.f, FColor::Black, TEXT("Check BossHPBar Once Again"));
			}
		}
	}
}

void APlayerControllerBase::CreateEndPopUp()
{
	if (!IsValid(EndPopUpWidgetClass))
	{
		return;
	}

	EndPopUpWidget = CreateWidget<UEndPopUpWidget>(this, EndPopUpWidgetClass);
	if (!IsValid(EndPopUpWidget))
	{
		return;
	}
}

void APlayerControllerBase::Server_RequestReward_Implementation(FName RewardItemRowName)
{
	if (const auto* PS = GetPlayerState<ARSPlayerState>())
	{
		if (UInventoryComponent* Inventory = PS->GetInventoryComponent())
		{
			Inventory->AddItem(RewardItemRowName, 1);
		}
	}

	if (const auto* PlayerCharacterBase = GetPawn<APlayerCharacterBase>())
	{
		RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: Get character"), *LOG_LINE_INFO);
		if (UInteractableComponent* Interactable = PlayerCharacterBase->GetCurrentFocusedInteractable())
		{
			RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: Try destroy interactable %s"), *LOG_LINE_INFO,
			             *Interactable->GetOwner()->GetName());
			Interactable->GetOwner()->Destroy();
		}
	}
}

void APlayerControllerBase::CloseRewardPopup()
{
	if (IsLocalController())
	{
		URewardPopupWidget* Widget = RewardPopupWidget.Get();
		if (IsValid(Widget))
		{
			Widget->RemoveFromParent();
			RewardPopupWidget.Reset();
		}
	}
}

void APlayerControllerBase::CreateRewardPopupWidget(const TArray<FName>& RewardItemRowNames)
{
	if (RewardItemRowNames.IsEmpty())
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: RewardItemRowNames is empty"), *LOG_LINE_INFO);
	}

	if (!IsValid(RewardPopupWidgetClass))
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: RewardPopupWidgetClass is not valid"), *LOG_LINE_INFO);
		return;
	}

	RewardPopupWidget = CreateWidget<URewardPopupWidget>(this, RewardPopupWidgetClass);
	if (!RewardPopupWidget.IsValid())
	{
		RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: RewardPopupWidget is not valid"), *LOG_LINE_INFO);
		return;
	}

	RS_LOG_DEBUG(LogRSInit, Log, TEXT("%s: RewardPopupWidget is created"), *LOG_LINE_INFO);

	URewardPopupWidget* PopupWidget = RewardPopupWidget.Get();
	PopupWidget->AddToViewport();

	PopupWidget->Init(RewardItemRowNames);
}
void APlayerControllerBase::CreatePauseWidget()
{
	if (!IsValid(PauseWidgetClass))
	{
		return;
	}

	auto* PauseWidget = CreateWidget<UPauseWidget>(this, PauseWidgetClass);
	if (!IsValid(PauseWidget))
	{
		return;
	}

	PauseWidget->AddToViewport();
	PauseWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void APlayerControllerBase::CreateSettingWidget()
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


void APlayerControllerBase::ShowEndPopUp()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!IsValid(EndPopUpWidget))
	{
		return;
	}

	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(EndPopUpWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	EndPopUpWidget->AddToViewport();
}

void APlayerControllerBase::Client_ShowRewardUI_Implementation(const TArray<FName>& RewardItemRowNames)
{
	RS_PRINT_SCREEN(1.f, FColor::Cyan, TEXT("RewardItemRowNames Num : %d"), RewardItemRowNames.Num());

	CreateRewardPopupWidget(RewardItemRowNames);
}


void APlayerControllerBase::Server_RequestSpectateTargets_Implementation()
{
	// 서버에서 호출하는 살아있는 사람들을 Array에 담아 클라이언트로 보내 그중 Controller의 View를 SetViewTarget으로 한다.
	TArray<APlayerCharacterBase*> AliveTargets;
	// FConstPlayerControllerIterator -> 서버에 있는 사람들의 컨트롤러를 반환해주는 이터레이터
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!IsValid(PC))
		{
			continue;
		}
		APlayerCharacterBase* Char = Cast<APlayerCharacterBase>(PC->GetPawn());
		if (IsValid(Char) && !Char->IsDead())
		{
			AliveTargets.Add(Char);
		}
	}
	RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("AliveTargets Num : %d"), AliveTargets.Num());
	if (AliveTargets.Num() > 0)
	{
		Client_ReceiveSpectateTargets(AliveTargets);
	}

	else
	{
		// -> 종료로직 필요
	}
}

void APlayerControllerBase::Client_ReceiveSpectateTargets_Implementation(
	const TArray<APlayerCharacterBase*>& Targets)
{
	SpectateTargets = Targets;
	if (SpectateTargets.Num() > 0)
	{
		CurrentSpectateIndex = 0;
		auto* Actor = SpectateTargets[CurrentSpectateIndex];
		// 액터로 변환해서 보내야 뷰타겟이 가능
		if (IsValid(Actor))
		{
			SetViewTargetWithBlend(Actor, 0.5f);
		}
	}
}

void APlayerControllerBase::SpectateNext()
{
	if (SpectateTargets.Num() <= 0)
		return;

	CurrentSpectateIndex = (CurrentSpectateIndex + 1) % SpectateTargets.Num();

	auto* Actor = SpectateTargets[CurrentSpectateIndex];
	if (IsValid(Actor))
	{
		SetViewTargetWithBlend(Actor, 0.5f);
	}
}

void APlayerControllerBase::SpectatePrev()
{
	if (SpectateTargets.Num() <= 0)
		return;

	CurrentSpectateIndex = (CurrentSpectateIndex - 1 + SpectateTargets.Num()) % SpectateTargets.Num();

	auto* Actor = SpectateTargets[CurrentSpectateIndex];
	if (IsValid(Actor))
	{
		SetViewTargetWithBlend(Actor, 0.5f);
	}
}


void APlayerControllerBase::CreateControlHelpWidget()
{
	if (!IsValid(ControlHelpWidgetClass))
	{
		return;
	}

	auto* ControlHelpWidget = CreateWidget<URSUserWidget>(this, ControlHelpWidgetClass);
	if (!IsValid(ControlHelpWidget))
	{
		return;
	}

	ControlHelpWidget->AddToViewport();

	const bool bShow = URSGameUserSettings::Get()->GetShowControlsHelp();
	ControlHelpWidget->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
