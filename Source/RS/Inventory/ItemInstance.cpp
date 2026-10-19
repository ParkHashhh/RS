// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "RS/Core/LogCategory.h"
#include "RS/Inventory/InventoryComponent.h"

UItemInstance::UItemInstance()
{

}

void UItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UItemInstance, ItemGUID);
	DOREPLIFETIME(UItemInstance, ItemRowHandle);
	DOREPLIFETIME(UItemInstance, ItemCount);
}

void UItemInstance::Initialize(UInventoryComponent* InOwner, const FGuid& InItemGUID,
	const FDataTableRowHandle& InItemRowHandle, int32 InItemCount)
{
	OwnerInventory = InOwner;
	ItemGUID = InItemGUID;
	ItemRowHandle = InItemRowHandle;
	ItemCount = InItemCount;
}

void UItemInstance::OnRep_ItemRowHandle()
{
	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: OnRep_ItemRowHandle, RowName: %s, Outer: %s"),
		*LOG_LINE_INFO, *ItemRowHandle.RowName.ToString(), *GetNameSafe(GetTypedOuter<UInventoryComponent>()));

	// OwnerInventory는 Transient라 서버에서 Initialize()로만 채워지고 클라의 복제본에는 없음.
	// Outer는 리플리케이트된 서브오브젝트에도 항상 유효하므로 이걸로 인벤토리 컴포넌트를 찾는다.
	if (UInventoryComponent* Owner = GetTypedOuter<UInventoryComponent>())
	{
		Owner->OnInventoryChangedDelegate.Broadcast();
	}
}

void UItemInstance::OnRep_ItemCount()
{
	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: OnRep_ItemCount, ItemCount: %d"), *LOG_LINE_INFO, ItemCount);

	if (UInventoryComponent* Owner = GetTypedOuter<UInventoryComponent>())
	{
		Owner->OnInventoryChangedDelegate.Broadcast();
	}
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
