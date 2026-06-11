// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillSlotWidget.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"

USkillSlotWidget::USkillSlotWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void USkillSlotWidget::SetCooldownTimeText(const FText& CooldownText) const
{
	CooldownTime->SetText(CooldownText);
}

void USkillSlotWidget::SetCooldownShadowRatio(float Ratio) const
{
	check(FMath::IsWithinInclusive(Ratio, 0.0f, 1.0f));

	CooldownShadow->SetRenderScale(FVector2D(1.0f, Ratio));
}

void USkillSlotWidget::SetSkillReady(bool bIsReady) const
{
	CooldownTime->SetVisibility(bIsReady ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	CooldownShadow->SetVisibility(bIsReady ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}
