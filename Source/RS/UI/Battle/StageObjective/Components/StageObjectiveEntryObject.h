// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "UObject/Object.h"
#include "RS/GameFramework/GameState/RSGameState.h"
#include "StageObjectiveEntryObject.generated.h"

struct FStageObjective;
/**
 *
 */
UCLASS()
class RS_API UStageObjectiveEntryObject : public UObject
{
	GENERATED_BODY()

public:
	FText Title;
	FText Description;

	float Current;
	float Goal;

	int32 Idx;

	void Initialize(const FStageObjective& Objective, const UDataSubsystem* DataSubsystem, int32 InIdx);
};
