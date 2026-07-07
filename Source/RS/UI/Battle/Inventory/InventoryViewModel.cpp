// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryViewModel.h"

#include "InventorySlotViewModel.h"
#include "RS/Inventory/InventoryComponent.h"

void UInventoryViewModel::Initialize(UInventoryComponent* InInventoryComponent)
{
	if (!IsValid(InInventoryComponent))
	{
		return;
	}

	InventoryComponent = InInventoryComponent;

	ItemAddedHandle = InventoryComponent->OnItemAddedDelegate.AddUObject(this, &UInventoryViewModel::HandleItemAdded);
	ItemChangedHandle = InventoryComponent->OnItemChangedDelegate.AddUObject(
		this, &UInventoryViewModel::HandleItemChanged);
	ItemRemovedHandle = InventoryComponent->OnItemRemovedDelegate.AddUObject(
		this, &UInventoryViewModel::HandleItemRemoved);

	for (const FInventoryEntry& Entry : InventoryComponent->GetEntries())
	{
		HandleItemAdded(Entry.Item);
	}
}

void UInventoryViewModel::ShutDown()
{
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnItemAddedDelegate.Remove(ItemAddedHandle);
		InventoryComponent->OnItemChangedDelegate.Remove(ItemChangedHandle);
		InventoryComponent->OnItemRemovedDelegate.Remove(ItemRemovedHandle);

		InventoryComponent = nullptr;
	}
}

void UInventoryViewModel::HandleItemAdded(const UItemInstance* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	auto* SlotVM = NewObject<UInventorySlotViewModel>(this);

	SlotVM->Initialize(const_cast<UItemInstance*>(Item));

	SlotViewModels.Add(SlotVM);

	SlotViewModelAddedDelegate.Broadcast(SlotVM);
}

void UInventoryViewModel::HandleItemChanged(const UItemInstance* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	int32 Index = FindSlotViewModelIndex(Item);
	if (Index == INDEX_NONE)
	{
		return;
	}

	SlotViewModels[Index]->RefreshFromItem();
}

void UInventoryViewModel::HandleItemRemoved(const UItemInstance* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	int32 Index = FindSlotViewModelIndex(Item);
	if (Index == INDEX_NONE)
	{
		return;
	}

	SlotViewModelRemovedDelegate.Broadcast(SlotViewModels[Index]);

	SlotViewModels.RemoveAt(Index);
}

int32 UInventoryViewModel::FindSlotViewModelIndex(const UItemInstance* Item) const
{
	if (!IsValid(Item))
	{
		return INDEX_NONE;
	}

	return SlotViewModels.IndexOfByPredicate(
		[Item](const UInventorySlotViewModel* SlotVM)
		{
			return SlotVM && SlotVM->GetItemInstance() == Item;
		});
}
