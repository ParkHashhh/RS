// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"

#include "InventorySlotWidget.h"
#include "Components/WrapBox.h"
#include "RS/Inventory/InventoryComponent.h"

UInventoryWidget::UInventoryWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UInventoryWidget::Init(UInventoryComponent* InventoryComponent)
{
	if (!InventoryComponent)
	{
		return;
	}

	if (CachedInventoryComponent.IsValid())
	{
		CachedInventoryComponent.Get()->OnItemChanged.Remove(OnInventoryItemChangedHandle);
	}

	CachedInventoryComponent = InventoryComponent;

	OnInventoryItemChangedHandle = InventoryComponent->OnItemChanged.AddUObject(this, &ThisClass::OnInventoryItemChanged);

	InnerInit();
}

void UInventoryWidget::NativeDestruct()
{
	if (CachedInventoryComponent.IsValid())
	{
		CachedInventoryComponent.Get()->OnItemChanged.Remove(OnInventoryItemChangedHandle);
	}

	Super::NativeDestruct();
}

void UInventoryWidget::OnInventoryItemChanged(FName ItemRowName, int32 ItemCount) const
{
	if (UInventorySlotWidget* ItemSlot = InventorySlots.FindRef(ItemRowName))
	{
		ItemSlot->SetItemCount(ItemCount);
	}
}

void UInventoryWidget::InnerInit()
{
	for (const FInventoryItem& Item : CachedInventoryComponent.Get()->GetInventoryItems())
	{
		if (!Item.ItemRowName.IsNone() && Item.Count > 0)
		{
			if (UInventorySlotWidget* ItemSlot = InventorySlots.FindRef(Item.ItemRowName))
			{
				ItemSlot->SetItemCount(Item.Count);
			}
			else
			{
				ItemSlot = CreateWidget<UInventorySlotWidget>(this, InventorySlotWidgetClass);
				if (ItemSlot)
				{
					ItemSlot->Init(Item.ItemRowName, Item.Count);
					SlotContainer->AddChildToWrapBox(ItemSlot);

					InventorySlots.Emplace(Item.ItemRowName, ItemSlot);
				}
			}
		}
	}
}
