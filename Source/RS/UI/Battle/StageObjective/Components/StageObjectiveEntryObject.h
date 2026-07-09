// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StageObjectiveEntryObject.generated.h"

struct FStageObjective;
/**
 *
 */
UCLASS()
class RS_API UStageObjectiveEntryObject : public UObject
{
	GENERATED_BODY()

	FText Title;
	FText Description;

	int Current;
	int Goal;

public:
	void Initialize(const FStageObjective& Objective);
};
