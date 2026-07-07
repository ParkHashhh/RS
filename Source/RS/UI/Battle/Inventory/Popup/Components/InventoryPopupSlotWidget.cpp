// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryPopupSlotWidget.h"

#include "InventoryPopupSlotObject.h"
#include "Components/TextBlock.h"
#include "RS/LogCategory.h"
#include "RS/Inventory/ItemInstance.h"

void UInventoryPopupSlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: NativeOnListItemObjectSet"), *LOG_LINE_INFO);

	const auto* SlotObj = Cast<UInventoryPopupSlotObject>(ListItemObject);
	if (!SlotObj)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: SlotObj is nullptr"), *LOG_LINE_INFO);
		return;
	}

	const UItemInstance* Item = SlotObj->GetItem();
	if (!Item)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Item is nullptr"), *LOG_LINE_INFO);
		return;
	}

	const FItemTableInfo* ItemInfo = Item->GetItemInfo();
	if (!ItemInfo)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: ItemInfo is nullptr"), *LOG_LINE_INFO);
		return;
	}

	SetItemCountText(Item->GetItemCount());

	if (ItemInfo->IconSprite)
	{
		SetIconSprite(ItemInfo->IconSprite);
		RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: NativeOnListItemObjectSet, Item: %s, IconSprite: %s, ItemCount: %d"),
				 *LOG_LINE_INFO, *ItemInfo->Name.ToString(), *ItemInfo->IconSprite->GetName(), Item->GetItemCount());
	}
	else if (ItemInfo->IconTexture)
	{
		SetIconTexture(ItemInfo->IconTexture);
		RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: NativeOnListItemObjectSet, Item: %s, IconTexture: %s, ItemCount: %d"),
				 *LOG_LINE_INFO, *ItemInfo->Name.ToString(), *ItemInfo->IconTexture->GetName(), Item->GetItemCount());
	}
}

void UInventoryPopupSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

void UInventoryPopupSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventoryPopupSlotWidget::SetItemCountText(int32 Count) const
{
	if (!ItemCountText)
	{
		return;
	}

	const bool bShow = Count > 0;
	if (bShow)
	{
		ItemCountText->SetText(FText::AsCultureInvariant(FString::FromInt(Count)));
	}

	ItemCountText->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
