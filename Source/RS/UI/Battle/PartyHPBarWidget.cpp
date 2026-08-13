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
#include "GameFramework/PlayerState.h"

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
	FName PMName = NAME_None;
	if (const APlayerState* PartyMemberPS = PartyMember->GetPlayerState<APlayerState>())
	{
		PMName = FName(*PartyMemberPS->GetPlayerName());
	}
	if (PMName.IsNone())
	{
		PMName = TEXT("Player");
	}

	//이름 첫 글자를 대문자로 변환한다.
	FString PMName_Str = PMName.ToString();
	FString PMName_First = PMName_Str.Left(1);
	FString PMName_Upper = PMName_First.ToUpper();

	//첫 글자가 알파벳인지 확인한다.
	FString FinalFirstLetter;
	if (FChar::IsAlpha(PMName_Upper[0]))
	{
		FinalFirstLetter = PMName_Upper;
	}
	else
	{
		FinalFirstLetter = FString(TEXT("P"));
	}

	//번호를 가져운다.
	FString PMNumber_Str = FString::FromInt(ListOrder);

	//첫 영문자와 번호를 조합한다.
	FString FinalName = FinalFirstLetter + PMNumber_Str;

	FText FinalName_Text = FText::FromString(FinalName);
	PartyMemberName->SetText(FinalName_Text);
}	




