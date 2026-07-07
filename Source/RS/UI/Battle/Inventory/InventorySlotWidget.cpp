// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"

#include "Popup/Components/InventoryPopupSlotObject.h"

void UInventorySlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const auto* SlotObj = Cast<UInventoryPopupSlotObject>(ListItemObject);
	if (!SlotObj)
	{
		return;
	}

	const UItemInstance* Item = SlotObj->GetItem();

	// TODO: 슬롯 세팅
}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}
