// Fill out your copyright notice in the Description page of Project Settings.


#include "BossHPBarWidget.h"
#include "TimerManager.h"
#include "Components/ProgressBar.h"
#include "RS/Core/LogCategory.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Zombie/Boss/ZombieBoss.h"
#include "RS/Player/GAS/Attribute/PlayerAttributeSet.h"
#include "RS/Zombie/GAS/ZombieBossAttributeSet.h"
#include "RS/GameFramework/GameState/RSGameState.h"
#include "Components/TextBlock.h"
#include "RS/Player/Subsystem/GameplaySubsystem.h"

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
			// 위젯이 GameplayReady보다 먼저 생성되는 경우가 많아서, 나중에 준비돼도 알림 받도록 바인딩해둠
			GameplaySubsystem->OnGameplayReady.AddUObject(this, &ThisClass::OnGameplayReadyChanged);

			if (GameplaySubsystem->IsGameplayReady())
			{
				SetBossInfo();
			}
		}
	}
}

void UBossHPBarWidget::OnGameplayReadyChanged(bool bGameplayReady)
{
	if (bGameplayReady)
	{
		SetBossInfo();
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
	UAbilitySystemComponent* BossASC = Boss ? Boss->GetAbilitySystemComponent() : nullptr;
	if (!Boss || !BossASC)
	{
		// CurrentBoss/ASC가 아직 리플리케이트 전일 수 있음 - 0.1초 뒤 재시도
		constexpr int32 MaxSetBossInfoRetryCount = 50;
		if (SetBossInfoRetryCount < MaxSetBossInfoRetryCount)
		{
			++SetBossInfoRetryCount;
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, this, &ThisClass::SetBossInfo, 0.1f, false);
		}
		return;
	}
	SetBossInfoRetryCount = 0;

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


