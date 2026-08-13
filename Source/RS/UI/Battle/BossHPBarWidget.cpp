// Fill out your copyright notice in the Description page of Project Settings.


#include "BossHPBarWidget.h"
#include "Components/ProgressBar.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Zombie/ZombieBoss.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/Zombie/GAS/ZombieBossAttributeSet.h"
#include "RS/GameMode/RSGameState.h"
#include "Components/TextBlock.h"
#include "RS/GameMode/GameplaySubsystem.h"

UBossHPBarWidget::UBossHPBarWidget(const FObjectInitializer& ObjectInitializer)
	: URSUserWidget(ObjectInitializer)
{
}

void UBossHPBarWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UBossHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			if (GameplaySubsystem->IsGameplayReady())
			{
				SetBossInfo();
			}
		}
	}
}

void UBossHPBarWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UBossHPBarWidget::SetVisibility(ESlateVisibility InVisibility)
{
	//보스 체력바 위젯을 화면에 띄워야 하는 경우에
	//보스 존재 여부 불리언을 추가로 확인한 후에 화면에 띄워야 한다.
	if (InVisibility == ESlateVisibility::Visible ||
		InVisibility == ESlateVisibility::HitTestInvisible ||
		InVisibility == ESlateVisibility::SelfHitTestInvisible)
	{
		if (GetWorld())
		{
			ARSGameState* GS = GetWorld()->GetGameState<ARSGameState>();
			if (GS)
			{
				AZombieBoss* CurrentBossPtr = GS->GetCurrentBoss();
				if (CurrentBossPtr != nullptr)
				{
					Super::SetVisibility(InVisibility);
				}
				else
				{
					Super::SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}
	}
	else
	{
		Super::SetVisibility(InVisibility);
	}
}

void UBossHPBarWidget::OnCurrHpChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: 보스 HP가 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, Data.NewValue);
	CurrHP = Data.NewValue;

	K2_OnHPChanged(MaxHP != 0 ? CurrHP / MaxHP : 0);
	if (BossHPBar)
	{
		BossHPBar->SetPercent(MaxHP != 0 ? CurrHP / MaxHP : 0.f);
	}
}

void UBossHPBarWidget::OnMaxHpChanged(const FOnAttributeChangeData& Data)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: 보스 Max HP가 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, Data.NewValue);
	MaxHP = Data.NewValue;

	K2_OnHPChanged(MaxHP != 0 ? CurrHP / MaxHP : 0);
	if (BossHPBar)
	{
		BossHPBar->SetPercent(MaxHP != 0 ? CurrHP / MaxHP : 0.f);
	}
}

void UBossHPBarWidget::SetBossInfo()
{
	//보스의 ASC와 연동해야 한다.
	ARSGameState* GameState = GetWorld()->GetGameState<ARSGameState>();
	if (!GameState)
	{
		return;
	}

	AZombieBoss* Boss = GameState->GetCurrentBoss();
	if (!Boss)
	{
		return;
	}

	UAbilitySystemComponent* BossASC = Boss->GetAbilitySystemComponent();
	if (!BossASC)
	{
		return;
	}

	FGameplayAttribute Attribute;
	FDelegateHandle Handle;
	FOnAttributeChangeData InitialData;

	// Current HP
	Attribute = UZombieBossAttributeSet::GetCurrentHPAttribute();
	Handle = BossASC->GetGameplayAttributeValueChangeDelegate(Attribute)
		.AddUObject(this, &ThisClass::OnCurrHpChanged);
	AttributeDelegateBindings.Emplace(Attribute, Handle);

	InitialData.Attribute = Attribute;
	InitialData.NewValue = BossASC->GetNumericAttribute(Attribute);
	InitialData.OldValue = InitialData.NewValue;
	OnCurrHpChanged(InitialData);

	// Max HP
	Attribute = UZombieBossAttributeSet::GetMaxHPAttribute();
	Handle = BossASC->GetGameplayAttributeValueChangeDelegate(Attribute)
		.AddUObject(this, &ThisClass::OnMaxHpChanged);
	AttributeDelegateBindings.Emplace(Attribute, Handle);

	InitialData.Attribute = Attribute;
	InitialData.NewValue = BossASC->GetNumericAttribute(Attribute);
	InitialData.OldValue = InitialData.NewValue;
	OnMaxHpChanged(InitialData);

	// 이름
	FName BZName = Boss->GetTableRowName();
	FText BZName_Text = FText::FromName(BZName);
	BossName->SetText(BZName_Text);
}


