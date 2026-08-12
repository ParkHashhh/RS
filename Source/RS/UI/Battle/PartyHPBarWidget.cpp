// Fill out your copyright notice in the Description page of Project Settings.


#include "PartyHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Zombie/ZombieBoss.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/Zombie/GAS/ZombieBossAttributeSet.h"
#include "RS/GameMode/RSGameState.h"
#include "Components/TextBlock.h"
#include "RS/GameMode/GameplaySubsystem.h"

UPartyHPBarWidget::UPartyHPBarWidget(const FObjectInitializer& ObjectInitializer)
	: URSUserWidget(ObjectInitializer)
{
}

void UPartyHPBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPartyHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			if (GameplaySubsystem->IsGameplayReady())
			{
				SetPartyMemberInfo();
			}
		}
	}
}

void UPartyHPBarWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPartyHPBarWidget::OnCurrHpChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: 파티원 HP가 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, Data.NewValue);
	RS_PRINT_SCREEN(5.f, FColor::Black, TEXT("파티원 체력 : %.2f"), Data.NewValue);
	CurrHP = Data.NewValue;

	K2_OnHPChanged(MaxHP != 0 ? CurrHP / MaxHP : 0);
	if (PartyMemberHPBar)
	{
		PartyMemberHPBar->SetPercent(MaxHP != 0 ? CurrHP / MaxHP : 0.f);
	}
}

void UPartyHPBarWidget::OnMaxHpChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: 파티원 Max HP가 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, Data.NewValue);
	RS_PRINT_SCREEN(5.f, FColor::Black, TEXT("파티원 최대 체력 : %.2f"), Data.NewValue);
	MaxHP = Data.NewValue;

	K2_OnHPChanged(MaxHP != 0 ? CurrHP / MaxHP : 0);
	if (PartyMemberHPBar)
	{
		PartyMemberHPBar->SetPercent(MaxHP != 0 ? CurrHP / MaxHP : 0.f);
	}
}

void UPartyHPBarWidget::SetPartyMemberInfo()
{
	if (!PartyMember)
	{
		RS_PRINT_SCREEN(5.f, FColor::Blue, TEXT("No Party Member"));
		return;
	}

	//파티원의 ASC와 연동해야 한다.
	UAbilitySystemComponent* PartyMemberASC = PartyMember->GetAbilitySystemComponent();
	if (!PartyMemberASC)
	{
		return;
	}

	FGameplayAttribute Attribute;
	FDelegateHandle Handle;
	FOnAttributeChangeData InitialData;

	// Current HP
	Attribute = UPlayerAttributeSet::GetCurrentHPAttribute();
	Handle = PartyMemberASC->GetGameplayAttributeValueChangeDelegate(Attribute)
		.AddUObject(this, &ThisClass::OnCurrHpChanged);
	AttributeDelegateBindings.Emplace(Attribute, Handle);

	InitialData.Attribute = Attribute;
	InitialData.NewValue = PartyMemberASC->GetNumericAttribute(Attribute);
	InitialData.OldValue = InitialData.NewValue;
	OnCurrHpChanged(InitialData);

	// Max HP
	Attribute = UPlayerAttributeSet::GetMaxHPAttribute();
	Handle = PartyMemberASC->GetGameplayAttributeValueChangeDelegate(Attribute)
		.AddUObject(this, &ThisClass::OnMaxHpChanged);
	AttributeDelegateBindings.Emplace(Attribute, Handle);

	InitialData.Attribute = Attribute;
	InitialData.NewValue = PartyMemberASC->GetNumericAttribute(Attribute);
	InitialData.OldValue = InitialData.NewValue;
	OnMaxHpChanged(InitialData);

	// 이름
	// 플레이어의 스팀 닉네임을 얻어온 후 첫 글자와 숫자를 조합해 파티원 체력바에 표시할 이름을 만들어야 한다.
	//FName BZName = Boss->GetTableRowName();
	//FText BZName_Text = FText::FromName(BZName);
	//BossName->SetText(BZName_Text);
}	




