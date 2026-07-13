// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RewardEntryObject.generated.h"

class UDataSubsystem;
class UStatEntryObject;
class UPaperSprite;
struct FItemTableInfo;
/**
 *
 */
UCLASS()
class RS_API URewardEntryObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<UPaperSprite> ItemIcon;

	UPROPERTY(Transient)
	FName ItemRowName;

	UPROPERTY(Transient)
	FText ItemName;

	UPROPERTY()
	TArray<TObjectPtr<UStatEntryObject>> StatEntryObjects;

	void Initialize(FName InItemRowName, const UDataSubsystem* DataSubsystem);
};
