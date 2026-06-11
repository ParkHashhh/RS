// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/RSUserWidget.h"
#include "BattleHUDWidget.generated.h"

/**
 *
 */
UCLASS()
class RS_API UBattleHUDWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UBattleHUDWidget(const FObjectInitializer& ObjectInitializer);
};
