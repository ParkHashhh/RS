// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoWidget.h"

#include "Components/Image.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"

void UAmmoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (AmmoBar)
	{
		AmmoBarMaterialInstance = AmmoBar->GetDynamicMaterial();
	}
}

bool UAmmoWidget::TryBindCharacterDelegates(APlayerCharacterBase* Character)
{
	if (!Super::TryBindCharacterDelegates(Character))
	{
		return false;
	}

	if (UAbilitySystemComponent* ASC = GetCachedCharacter()->GetAbilitySystemComponent())
	{
		FGameplayAttribute Attribute;
		FDelegateHandle Handle;
		FOnAttributeChangeData InitialData;

		// Current HP
		Attribute = UPlayerAttributeSet::GetCurrAmmoAttribute();
		Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
					.AddUObject(this, &ThisClass::OnCurrAmmoChanged);
		AttributeDelegateBindings.Emplace(Attribute, Handle);

		InitialData.Attribute = Attribute;
		InitialData.NewValue = ASC->GetNumericAttribute(Attribute);
		InitialData.OldValue = InitialData.NewValue;
		OnCurrAmmoChanged(InitialData);

		// Max HP
		Attribute = UPlayerAttributeSet::GetMaxAmmoAttribute();
		Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
					.AddUObject(this, &ThisClass::OnMaxAmmoChanged);
		AttributeDelegateBindings.Emplace(Attribute, Handle);

		InitialData.Attribute = Attribute;
		InitialData.NewValue = ASC->GetNumericAttribute(Attribute);
		InitialData.OldValue = InitialData.NewValue;
		OnMaxAmmoChanged(InitialData);
	}

	return true;;
}

void UAmmoWidget::OnCurrAmmoChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: Ammo가 변경됨! 현재 값: %d"), *LOG_LINE_INFO, (int32)Data.NewValue);
	CurrAmmo = Data.NewValue;

	K2_OnAmmoChanged(MaxAmmo != 0 ? CurrAmmo / MaxAmmo : 0);
}

void UAmmoWidget::OnMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: Max Ammo 변경됨! 현재 값: %d"), *LOG_LINE_INFO, (int32)Data.NewValue);
	MaxAmmo = Data.NewValue;

	K2_OnAmmoChanged(MaxAmmo != 0 ? CurrAmmo / MaxAmmo : 0);
}
