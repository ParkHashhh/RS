// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryPopupSlotWidget.h"

#include "InventoryPopupSlotObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RS/Core/LogCategory.h"
#include "RS/Inventory/ItemInstance.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/UI/Colony/Result/Components/StageResultItemSlotObject.h"

void UInventoryPopupSlotWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const auto* SlotObj = Cast<UInventoryPopupSlotObject>(ListItemObject);
	if (!SlotObj)
	{
		if (const auto* ResultSlotObj = Cast<UStageResultItemSlotObject>(ListItemObject))
		{
			SetResultItemSlot(ResultSlotObj);
			return;
		}

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
		GetIconImage()->SetVisibility(ESlateVisibility::Collapsed);
		ItemCountText->SetVisibility(ESlateVisibility::Collapsed);
		SetBorderVisible(false);
		// RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: ItemInfo is nullptr"), *LOG_LINE_INFO);
		return;
	}
	else
	{
		GetIconImage()->SetVisibility(ESlateVisibility::Visible);
		ItemCountText->SetVisibility(ESlateVisibility::Visible);
	}

	SetItemCountText(Item->GetItemCount());
	SetBorderVisible(true);
	SetGradeBorderColor(ItemInfo->Grade);

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
	else
	{
		RS_LOG_DEBUG(LogRSInventory, Error,
			TEXT("%s: NativeOnListItemObjectSet, Item: %s - IconSprite AND IconTexture are BOTH null on ItemInfo row"),
			*LOG_LINE_INFO, *ItemInfo->Name.ToString());
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

void UInventoryPopupSlotWidget::SetResultItemSlot(const UStageResultItemSlotObject* ResultSlotObj)
{
	// 아이콘은 RowName만으로 결정되는 정적 데이터라 UItemInstance(리플리케이트되는 서브오브젝트)를
	// 기다릴 필요가 없음 - 원격 클라에서 서브오브젝트 replication이 늦거나 유실돼도 즉시 그릴 수 있게
	// DataTable에서 바로 조회한다.
	const UGameInstance* GI = GetGameInstance();
	const UDataSubsystem* DataSubsystem = GI ? GI->GetSubsystem<UDataSubsystem>() : nullptr;
	const UDataTable* ItemTable = DataSubsystem ? DataSubsystem->GetItemInfoTable() : nullptr;

	const FItemTableInfo* ItemInfo = ItemTable
		? ItemTable->FindRow<FItemTableInfo>(ResultSlotObj->GetItemRowName(), TEXT("SetResultItemSlot"))
		: nullptr;

	if (!ItemInfo)
	{
		GetIconImage()->SetVisibility(ESlateVisibility::Collapsed);
		ItemCountText->SetVisibility(ESlateVisibility::Collapsed);
		SetBorderVisible(false);
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: SetResultItemSlot, ItemInfo is nullptr, Row: %s"),
			*LOG_LINE_INFO, *ResultSlotObj->GetItemRowName().ToString());
		return;
	}
	else
	{
		GetIconImage()->SetVisibility(ESlateVisibility::Visible);
		ItemCountText->SetVisibility(ESlateVisibility::Visible);
	}

	SetItemCountText(ResultSlotObj->GetItemCount());
	SetBorderVisible(true);
	SetGradeBorderColor(ItemInfo->Grade);

	if (ItemInfo->IconSprite)
	{
		SetIconSprite(ItemInfo->IconSprite);
		RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: SetResultItemSlot, Row: %s, IconSprite: %s, ItemCount: %d"),
				 *LOG_LINE_INFO, *ItemInfo->Name.ToString(), *ItemInfo->IconSprite->GetName(), ResultSlotObj->GetItemCount());
	}
	else if (ItemInfo->IconTexture)
	{
		SetIconTexture(ItemInfo->IconTexture);
		RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: SetResultItemSlot, Row: %s, IconTexture: %s, ItemCount: %d"),
				 *LOG_LINE_INFO, *ItemInfo->Name.ToString(), *ItemInfo->IconTexture->GetName(), ResultSlotObj->GetItemCount());
	}
	else
	{
		RS_LOG_DEBUG(LogRSInventory, Error,
			TEXT("%s: SetResultItemSlot, Row: %s - IconSprite AND IconTexture are BOTH null on ItemInfo row"),
			*LOG_LINE_INFO, *ItemInfo->Name.ToString());
	}
}
