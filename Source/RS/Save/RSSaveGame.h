// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSTypes.h"
#include "GameFramework/SaveGame.h"
#include "RSSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class RS_API URSSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FItemStorageInfo> SavedInventory;

	UPROPERTY()
	FName SavedTextureName;

	UPROPERTY()
	FName SavedSkillName;

	UPROPERTY()
	int32 SavedDifficultyStage = 1;
};
