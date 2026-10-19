// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/UITypes.h"
#include "Blueprint/UserWidget.h"
#include "SlotWidget.generated.h"

class UIconWidgetBase;
/**
 *
 */
UCLASS()
class RS_API USlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USlotWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UIconWidgetBase> IconWidget;
};
