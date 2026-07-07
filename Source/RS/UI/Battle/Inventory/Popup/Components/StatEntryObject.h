// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "StatEntryObject.generated.h"

class UPaperSprite;
/**
 *
 */
UCLASS()
class RS_API UStatEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FText StatName;

	UPROPERTY()
	FText StatDescription;

	UPROPERTY()
	TObjectPtr<UPaperSprite> Icon;

	void Initialize(const FText& InStatName, const FText& InStatDescription, UPaperSprite* InIcon);
};
