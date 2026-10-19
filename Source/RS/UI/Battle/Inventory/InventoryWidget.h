// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "InventoryWidget.generated.h"

class UInventorySlotViewModel;
class UInventoryViewModel;
class UWrapBox;
class UInventorySlotWidget;
class UInventoryComponent;
/**
 *
 */
UCLASS()
class RS_API UInventoryWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UInventoryWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(Transient)
	TObjectPtr<UInventoryViewModel> InventoryViewModel;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> SlotContainer;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UInventorySlotViewModel>, TObjectPtr<UInventorySlotWidget>> InventorySlots;

	UPROPERTY(EditDefaultsOnly, Category="Inventory", meta=(AllowPrivateAccess=true))
	TSubclassOf<UInventorySlotWidget> InventorySlotWidgetClass;

	FDelegateHandle SlotAddedHandle;
	FDelegateHandle SlotRemovedHandle;

public:
	void Init(UInventoryViewModel* InViewModel);

	FORCEINLINE UInventoryViewModel* GetInventoryViewModel() const
	{
		return InventoryViewModel;
	}

	FORCEINLINE TMap<TObjectPtr<UInventorySlotViewModel>, TObjectPtr<UInventorySlotWidget>>& GetInventorySlots()
	{
		return InventorySlots;
	}

protected:
	virtual void NativeDestruct() override;

	void HandleSlotAdded(UInventorySlotViewModel* SlotViewModel);
	void HandleSlotRemoved(UInventorySlotViewModel* SlotViewModel);
};
