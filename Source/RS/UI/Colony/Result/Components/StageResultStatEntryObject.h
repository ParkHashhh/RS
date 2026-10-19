// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Core/RSTypes.h"
#include "UObject/Object.h"
#include "StageResultStatEntryObject.generated.h"

/**
 *
 */
UCLASS()
class RS_API UStageResultStatEntryObject : public UObject
{
	GENERATED_BODY()

private:
	EStageResultStatType StatType;
	float StatValue;

public:
	void Initialize(EStageResultStatType InStatType, float InStatValue);

	FText GetStatNameText() const;

	FText GetStatValueText() const;
};
