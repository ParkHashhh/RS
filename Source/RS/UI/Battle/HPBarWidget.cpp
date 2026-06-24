// Fill out your copyright notice in the Description page of Project Settings.


#include "HPBarWidget.h"

#include "Components/Image.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"

UHPBarWidget::UHPBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHPBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (HPBar)
	{
		HPBarMaterialInstance = HPBar->GetDynamicMaterial();
	}
}

bool UHPBarWidget::TryBindCharacterDelegates(APlayerCharacterBase* Character)
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
		Attribute = UPlayerAttributeSet::GetCurrentHPAttribute();
		Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
		            .AddUObject(this, &ThisClass::OnCurrHpChanged);
		AttributeDelegateBindings.Emplace(Attribute, Handle);

		InitialData.Attribute = Attribute;
		InitialData.NewValue = ASC->GetNumericAttribute(Attribute);
		InitialData.OldValue = InitialData.NewValue;
		OnCurrHpChanged(InitialData);

		// Max HP
		Attribute = UPlayerAttributeSet::GetMaxHPAttribute();
		Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
		            .AddUObject(this, &ThisClass::OnMaxHpChanged);
		AttributeDelegateBindings.Emplace(Attribute, Handle);

		InitialData.Attribute = Attribute;
		InitialData.NewValue = ASC->GetNumericAttribute(Attribute);
		InitialData.OldValue = InitialData.NewValue;
		OnMaxHpChanged(InitialData);
	}

	return true;
}

void UHPBarWidget::UnbindCharacterDelegates()
{
	Super::UnbindCharacterDelegates();
}

void UHPBarWidget::OnCurrHpChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: HP가 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, Data.NewValue);
	CurrHP = Data.NewValue;

	K2_OnHPChanged(MaxHP != 0 ? CurrHP / MaxHP : 0);
}

void UHPBarWidget::OnMaxHpChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: Max HP가 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, Data.NewValue);
	MaxHP = Data.NewValue;

	K2_OnHPChanged(MaxHP != 0 ? CurrHP / MaxHP : 0);
}
