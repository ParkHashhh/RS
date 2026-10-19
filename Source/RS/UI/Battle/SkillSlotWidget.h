// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/SlotWidget.h"
#include "SkillSlotWidget.generated.h"

class UBorder;
class UTextBlock;
/**
 *
 */
UCLASS()
class RS_API USkillSlotWidget : public USlotWidget
{
	GENERATED_BODY()

public:
	USkillSlotWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CooldownTime;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> CooldownShadow;

public:
	void SetCooldownTimeText(const FText& CooldownText) const;
	void SetCooldownShadowRatio(float Ratio) const;
	void SetSkillReady(bool bIsReady) const;
};
