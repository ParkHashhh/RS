// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "InventorySlotViewModel.generated.h"

class UItemInstance;
/**
 *
 */
UCLASS()
class RS_API UInventorySlotViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TObjectPtr<UItemInstance> ItemInstance;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter=GetIcon, meta=(AllowPrivateAccess=true))
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter=GetCountText, meta=(AllowPrivateAccess=true))
	FText CountText;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter=GetShowCount, meta=(AllowPrivateAccess=true))
	bool bShowCount;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter=GetItemName, meta=(AllowPrivateAccess=true))
	FText ItemName;

public:
	void Initialize(UItemInstance* InItem);

	void RefreshFromItem();

	UItemInstance* GetItemInstance() const
	{
		return ItemInstance;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	UTexture2D* GetIcon() const
	{
		return Icon;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	const FText& GetCountText() const
	{
		return CountText;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	bool GetShowCount() const
	{
		return bShowCount;
	}

	UFUNCTION(BlueprintPure, FieldNotify)
	const FText& GetItemName() const
	{
		return ItemName;
	}
};
