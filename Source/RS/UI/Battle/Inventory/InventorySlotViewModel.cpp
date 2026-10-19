// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotViewModel.h"

#include "RS/Core/RSUtils.h"
#include "RS/Inventory/ItemInstance.h"

void UInventorySlotViewModel::Initialize(UItemInstance* InItem)
{
	ItemInstance = InItem;

	RefreshFromItem();
}

void UInventorySlotViewModel::RefreshFromItem()
{
	if (!ItemInstance)
	{
		return;
	}

	const FItemTableInfo* ItemInfo = RSUtils::GetTableInfo<FItemTableInfo>(ItemInstance->GetItemRowHandle());
	if (!ItemInfo)
	{
		return;
	}

	UE_MVVM_SET_PROPERTY_VALUE(Icon, ItemInfo->IconTexture);

	bool bShow = ItemInstance->GetItemCount() > 1;
	UE_MVVM_SET_PROPERTY_VALUE(bShowCount, bShow);
	UE_MVVM_SET_PROPERTY_VALUE(CountText, bShow ? FText::AsNumber(ItemInstance->GetItemCount()) : FText::GetEmpty());

	UE_MVVM_SET_PROPERTY_VALUE(ItemName, ItemInfo->Name);
}
