// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"

#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"

UInventorySlotWidget::UInventorySlotWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UInventorySlotWidget::Init(FName NewItemRowName, int32 NewItemCount)
{
	if (NewItemRowName.IsNone() || NewItemCount <= 0)
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Invalid item row name or item count. row name: %s, count: %d"),
		             *LOG_LINE_INFO, *NewItemRowName.ToString(), NewItemCount);
		return;
	}

	ItemRowName = NewItemRowName;
	ItemCount = NewItemCount;

	InnerSetItem();
}

void UInventorySlotWidget::SetItemCount(int32 NewItemCount)
{
	if (NewItemCount <= 0)
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Invalid item count. count: %d"), *LOG_LINE_INFO, NewItemCount);
		return;
	}

	ItemCount = NewItemCount;

	InnerSetItemCount(NewItemCount);
}

void UInventorySlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UInventorySlotWidget::InnerSetItem()
{
	UGameInstance* GI = GetWorld()->GetGameInstance();
	auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>();
	DataSubsystem->ExecuteOrBindOnItemInfoLoaded(this, &ThisClass::OnItemTableLoaded);
}

void UInventorySlotWidget::InnerSetItemCount(int32 NewItemCount)
{
}

void UInventorySlotWidget::OnItemTableLoaded()
{
}
