// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "SelectSlotWidget.h"
#include "SelectUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class RS_API USelectUserWidget : public URSUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USelectSlotWidget> SlotWidget;

public:
	USelectUserWidget(const FObjectInitializer& ObjectInitializer);

public:
	void PickSelectPawn(class ASelectPawn* SelectPawn);
};
