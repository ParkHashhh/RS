// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryPopupSlotObject.h"

#include "RS/Inventory/InventoryComponent.h"

void UInventoryPopupSlotObject::Initialize(UItemInstance* InItem)
{
	Item = InItem;
}
