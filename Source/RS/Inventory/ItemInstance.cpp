// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInstance.h"

#include "RS/LogCategory.h"
#include "RS/Inventory/InventoryComponent.h"

UItemInstance::UItemInstance()
{
}

void UItemInstance::Initialize(UInventoryComponent* InOwner, const FGuid& InItemGUID,
	const FDataTableRowHandle& InItemRowHandle, int32 InItemCount)
{
	OwnerInventory = InOwner;
	ItemGUID = InItemGUID;
	ItemRowHandle = InItemRowHandle;
	ItemCount = InItemCount;
}

const FItemTableInfo* UItemInstance::GetItemInfo() const
{
	if (ItemRowHandle.IsNull())
	{
		return nullptr;
	}

	return ItemRowHandle.GetRow<FItemTableInfo>(TEXT("UItemInstance::GetItemInfo"));
}

FItemTableInfo UItemInstance::K2_GetItemInfo() const
{
	return *GetItemInfo();
}

void UItemInstance::SetItemInfo(FName ItemRowName, const UDataTable* DataTable)
{
	if (!DataTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: DataTable is null"), *LOG_LINE_INFO);
		return;
	}

	if (!DataTable->FindRow<FItemTableInfo>(ItemRowName, TEXT("UItemInstance::SetItemInfo")))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Item row not found: %s"), *LOG_LINE_INFO, *ItemRowName.ToString());
		return;
	}

	ItemRowHandle.DataTable = DataTable;
	ItemRowHandle.RowName = ItemRowName;
}
