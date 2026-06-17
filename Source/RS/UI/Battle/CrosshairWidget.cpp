// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairWidget.h"

#include "AbilitySystemComponent.h"
#include "RS/GameMode/GameplaySubsystem.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"

UCrosshairWidget::UCrosshairWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UCrosshairWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			GameplaySubsystem->OnGameplayReady.AddUObject(this, &ThisClass::OnGameplayReady);
		}
	}
}

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			if (GameplaySubsystem->IsGameplayReady())
			{
				OnGameplayReady(true);
			}
		}
	}
}

void UCrosshairWidget::NativeDestruct()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			GameplaySubsystem->OnGameplayReady.RemoveAll(this);
		}
	}

	UnbindCharacterDelegates();

	Super::NativeDestruct();
}

void UCrosshairWidget::OnGameplayReady(bool bIsReady)
{
	if (bIsReady)
	{
		if (const ULocalPlayer* LP = GetOwningLocalPlayer())
		{
			if (const auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
			{
				BindCharacterDelegates(GameplaySubsystem->GetCachedCharacter());
			}
		}
	}
	else
	{
		UnbindCharacterDelegates();
	}
}

void UCrosshairWidget::BindCharacterDelegates(APlayerCharacterBase* Character)
{
	if (!Character || CachedCharacter == Character)
	{
		return;
	}

	// 기존 바인딩 해제
	UnbindCharacterDelegates();

	CachedCharacter = Character;

	// 1. 일반 델리게이트 바인딩
	// GetDelegateHandles().Emplace(Character->델리게이트변수.AddUObject(this, &ThisClass::콜백));

	// 2. GAS 속성 변경 바인딩
	if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		FGameplayAttribute Attribute = UPlayerAttributeSet::GetShotSpreadRadiusAttribute();
		FDelegateHandle Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
		                            .AddUObject(this, &ThisClass::OnShotSpreadRadiusChanged);
		AttributeDelegateBindings.Emplace(Attribute, Handle);

		FOnAttributeChangeData InitialData;
		InitialData.Attribute = Attribute;
		InitialData.NewValue = ASC->GetNumericAttribute(Attribute);
		InitialData.OldValue = InitialData.NewValue;
		OnShotSpreadRadiusChanged(InitialData);
	}
}

void UCrosshairWidget::UnbindCharacterDelegates()
{
	if (CachedCharacter.IsValid())
	{
		// 1. 일반 델리게이트 해제
		// CachedCharacter->델리게이트변수.RemoveAll(this);

		// 2. GAS 속성 변경 해제
		if (UAbilitySystemComponent* ASC = CachedCharacter->GetAbilitySystemComponent())
		{
			for (auto& [Attribute, Handle] : AttributeDelegateBindings)
			{
				ASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
			}
		}
	}

	AttributeDelegateBindings.Empty();
	CachedCharacter.Reset();
}

void UCrosshairWidget::OnShotSpreadRadiusChanged(const FOnAttributeChangeData& Data)
{
	K2_OnShotSpreadRadiusChanged(Data.NewValue);
}
