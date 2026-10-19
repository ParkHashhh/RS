// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/SlotWidget.h"
#include "AuraSlotWidget.generated.h"

class UTextBlock;
/**
 *
 */
UCLASS()
class RS_API UAuraSlotWidget : public USlotWidget
{
	GENERATED_BODY()

public:
	UAuraSlotWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DurationText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StackCountText;
};
