// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"

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
