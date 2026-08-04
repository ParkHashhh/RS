// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StageResultItemSlotObject.generated.h"

struct FItemStorageInfo;
/**
 *
 */
UCLASS()
class RS_API UStageResultItemSlotObject : public UObject
{
	GENERATED_BODY()

private:
	FName ItemRowName;
	int32 ItemCount;

public:
	void Initialize(const FItemStorageInfo& InItem);

	FName GetItemRowName() const
	{
		return ItemRowName;
	}
	int32 GetItemCount() const
	{
		return ItemCount;
	}
};
