// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "InventoryPopupSlotObject.generated.h"

class UItemInstance;
/**
 *
 */
UCLASS()
class RS_API UInventoryPopupSlotObject : public UObject
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UItemInstance> Item = nullptr;

public:
	void Initialize(UItemInstance* InItem);

	UFUNCTION(BlueprintPure)
	UItemInstance* GetItem() const
	{
		return Item;
	}
};
