// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleHUDWidget.h"

#include "CrosshairWidget.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/GameplaySubsystem.h"

UBattleHUDWidget::UBattleHUDWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UBattleHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* GameplaySubsystem = GetOwningLocalPlayer()->GetSubsystem<UGameplaySubsystem>();
	check(GameplaySubsystem);

	GameplaySubsystem->OnGameplayReady.AddUObject(this, &ThisClass::OnGameplayReadyChanged);

	OnGameplayReadyChanged(GameplaySubsystem->IsGameplayReady());
}

void UBattleHUDWidget::OnGameplayReadyChanged(bool bGameplayReady) const
{
	if (!IsValid(GetCrosshairWidget()))
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: CrosshairWidget is not valid"), *LOG_LINE_INFO);
		return;
	}

	RS_LOG_DEBUG(LogRSUI, Log, TEXT("%s: CrosshairWidget visibility set to %s"), *LOG_LINE_INFO, bGameplayReady ? TEXT("HitTestInvisible") : TEXT("Collapsed"));
	GetCrosshairWidget()->SetVisibility(bGameplayReady ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
