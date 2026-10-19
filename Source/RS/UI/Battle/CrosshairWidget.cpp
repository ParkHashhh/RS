// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairWidget.h"

#include "RS/Player/Character/PlayerCharacterBase.h"

UCrosshairWidget::UCrosshairWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

bool UCrosshairWidget::TryBindCharacterDelegates(APlayerCharacterBase* Character)
{
	if (!Super::TryBindCharacterDelegates(Character))
	{
		return false;
	}

	GetCachedCharacter()->OnAimAlphaChanged.AddUObject(this, &ThisClass::OnAimAlphaChanged);

	return true;
}

void UCrosshairWidget::UnbindCharacterDelegates()
{
	if (GetCachedCharacter())
	{
		GetCachedCharacter()->OnAimAlphaChanged.RemoveAll(this);
	}

	Super::UnbindCharacterDelegates();
}

void UCrosshairWidget::OnAimAlphaChanged(float Alpha)
{
	K2_OnAimAlphaChanged(Alpha);
}
