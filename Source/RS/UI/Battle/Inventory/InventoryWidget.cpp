// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"

#include "InventorySlotViewModel.h"
#include "InventorySlotWidget.h"
#include "InventoryViewModel.h"
#include "Components/WrapBox.h"

UInventoryWidget::UInventoryWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UInventoryWidget::Init(UInventoryViewModel* InViewModel)
{
	if (!IsValid(InViewModel))
	{
		return;
	}

	InventoryViewModel = InViewModel;

	SlotAddedHandle = InventoryViewModel->GetSlotViewModelAddedDelegate().AddUObject(this, &ThisClass::HandleSlotAdded);
	SlotRemovedHandle = InventoryViewModel->GetSlotViewModelRemovedDelegate().AddUObject(this, &ThisClass::HandleSlotRemoved);

	for (auto& SlotViewModel : InventoryViewModel->GetSlotViewModels())
	{
		HandleSlotAdded(SlotViewModel);
	}
}

void UInventoryWidget::NativeDestruct()
{
	if (InventoryViewModel)
	{
		InventoryViewModel->GetSlotViewModelAddedDelegate().Remove(SlotAddedHandle);
		InventoryViewModel->GetSlotViewModelRemovedDelegate().Remove(SlotRemovedHandle);
	}

	for (auto& [Key, InventorySlot] : InventorySlots)
	{
		InventorySlot->RemoveFromParent();
	}

	InventorySlots.Empty();

	Super::NativeDestruct();
}

void UInventoryWidget::HandleSlotAdded(UInventorySlotViewModel* SlotViewModel)
{
	if (!IsValid(SlotViewModel))
	{
		return;
	}

	if (InventorySlots.Contains(SlotViewModel))
	{
		return;
	}
}

void UInventoryWidget::HandleSlotRemoved(UInventorySlotViewModel* SlotViewModel)
{
	if (!IsValid(SlotViewModel))
	{
		return;
	}
}
