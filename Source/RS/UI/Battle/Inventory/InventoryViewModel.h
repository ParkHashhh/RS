// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "InventoryViewModel.generated.h"

class UInventorySlotViewModel;
class UItemInstance;
class UInventoryComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotViewModelAdded, UInventorySlotViewModel*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotViewModelRemoved, UInventorySlotViewModel*);

/**
 *
 */
UCLASS()
class RS_API UInventoryViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UInventorySlotViewModel>> SlotViewModels;

	FOnSlotViewModelAdded SlotViewModelAddedDelegate;
	FOnSlotViewModelRemoved SlotViewModelRemovedDelegate;

	FDelegateHandle ItemAddedHandle;
	FDelegateHandle ItemChangedHandle;
	FDelegateHandle ItemRemovedHandle;

public:
	void Initialize(UInventoryComponent* InInventoryComponent);

	void ShutDown();

	const TArray<TObjectPtr<UInventorySlotViewModel>>& GetSlotViewModels() const
	{
		return SlotViewModels;
	}

	FORCEINLINE FOnSlotViewModelAdded& GetSlotViewModelAddedDelegate()
	{
		return SlotViewModelAddedDelegate;
	}

	FORCEINLINE FOnSlotViewModelRemoved& GetSlotViewModelRemovedDelegate()
	{
		return SlotViewModelRemovedDelegate;
	}

private:
	void HandleItemAdded(const UItemInstance* Item);
	void HandleItemChanged(const UItemInstance* Item);
	void HandleItemRemoved(const UItemInstance* Item);

	int32 FindSlotViewModelIndex(const UItemInstance* Item) const;
};
