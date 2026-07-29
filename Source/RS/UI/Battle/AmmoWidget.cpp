// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "RS/LogCategory.h"
#include "RS/RSConstants.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"

void UAmmoWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
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
		Attribute = UPlayerAttributeSet::GetCurrentAmmoAttribute();
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

	OnAmmoChanged();
}

void UAmmoWidget::OnMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: Max Ammo 변경됨! 현재 값: %d"), *LOG_LINE_INFO, (int32)Data.NewValue);
	MaxAmmo = Data.NewValue;

	int32 IntMaxAmmo = FMath::RoundToInt32(MaxAmmo);

	bool bDrawDivisionLine = IntMaxAmmo < HideDivisionLineCount;
	UMaterialInterface* Selected = bDrawDivisionLine ? MI_UseDivisions_True : MI_UseDivisions_False;
	if (CurrentMI != Selected)
	{
		CurrentMI = Selected;

		AmmoBarMaterialInstance = UMaterialInstanceDynamic::Create(Selected, this);

		RS_LOG_DEBUG(LogRSUI, Log, TEXT("%s: Ammo Widget MI changed."), *LOG_LINE_INFO);
	}

	if (AmmoBarMaterialInstance)
	{
		AmmoBarMaterialInstance->SetScalarParameterValue(FName(TEXT("DivisionsShift")),
		                                                 RSUtils::IsEven(IntMaxAmmo) ? 0.5f : 0.0f);
		if (bDrawDivisionLine)
		{
			AmmoBarMaterialInstance->SetScalarParameterValue(FName(TEXT("DivisionsCount")), MaxAmmo);
		}
	}

	OnAmmoChanged();
}

void UAmmoWidget::OnAmmoChanged()
{
	RefreshWidget();
}

void UAmmoWidget::RefreshWidget()
{
	if (FMath::IsNearlyZero(MaxAmmo))
	{
		RS_LOG_DEBUG(LogRSUI, Log, TEXT("%s: MaxAmmo is 0"), *LOG_LINE_INFO);

		if (AmmoBarMaterialInstance)
		{
			AmmoBarMaterialInstance->SetScalarParameterValue(ProgressParamName, 1.0f);
		}

		if (AmmoText)
		{
			AmmoText->SetText(FText::FromString(TEXT("- / -")));
		}

		return;
	}

	RS_LOG_DEBUG(LogRSUI, Log, TEXT("%s: CurrAmmo: %.0f, MaxAmmo: %.0f"), *LOG_LINE_INFO, CurrAmmo, MaxAmmo);

	if (AmmoBarMaterialInstance)
	{
		AmmoBarMaterialInstance->SetScalarParameterValue(ProgressParamName, CurrAmmo / MaxAmmo);
	}

	if (AmmoText)
	{
		AmmoText->SetText(FText::Format(
			FText::FromString(TEXT("{0} / {1}")),
			FMath::RoundToInt32(CurrAmmo), FMath::RoundToInt32(MaxAmmo)));
	}
}
