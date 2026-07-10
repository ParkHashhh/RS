// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "PlayerCharacterBase.h"
#include "RS/LogCategory.h"
#include "RS/UI/Battle/BattleHUDWidget.h"

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

	BattleHUDWidget->AddToViewport();
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

void APlayerControllerBase::Client_ReceiveSpectateTargets_Implementation(const TArray<class APlayerCharacterBase*>& Targets)
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
