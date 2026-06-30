// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"
#include "PlayerCharacterBase.h"
#include "RS/LogCategory.h"
#include "RS/UI/Battle/BattleHUDWidget.h"

APlayerControllerBase::APlayerControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 마우스 커서를 보이게 한다.
	bShowMouseCursor = false;
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		// 입력모드를 세팅한다
		// FInputModeGameOnly
		// FInputModeGameAndUI
		// FInputModeUIOnly

		FInputModeGameAndUI InputMode;
		SetInputMode(InputMode);

		CreateBattleHUD();
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


void APlayerControllerBase::Server_RequestSpectateTargets_Implementation()
{
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

		// 액터로 변환해서 보내야 뷰타겟이 가능
		if (APlayerController* TargetPC = Cast<APlayerController>(SpectateTargets[0]->GetController()))
		{
			SetControlRotation(TargetPC->GetControlRotation());
		}

		FViewTargetTransitionParams Params;
		Params.BlendTime = 0.5f;
		Params.BlendFunction = VTBlend_Linear; 

		SetViewTarget(SpectateTargets[0], Params);
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
		FViewTargetTransitionParams Params;
		SetViewTarget(Actor, Params);
		SetControlRotation(Actor->GetActorRotation());
		SetViewTarget(Actor);
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
		FViewTargetTransitionParams Params;
		SetViewTarget(Actor, Params);
		SetControlRotation(Actor->GetActorRotation());
		SetViewTarget(Actor);
	}
}
