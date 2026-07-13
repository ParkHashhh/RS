// Fill out your copyright notice in the Description page of Project Settings.


#include "RewardEntryWidget.h"

#include "RewardEntryObject.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "RS/UI/Battle/Inventory/Popup/Components/StatEntryObject.h"
#include "PaperSprite.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerControllerBase.h"

URewardEntryWidget::URewardEntryWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void URewardEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const auto* EntryObj = Cast<URewardEntryObject>(ListItemObject);
	if (!EntryObj)
	{
		return;
	}

	ItemRowName = EntryObj->ItemRowName;

	IconImage->SetBrushFromAtlasInterface(EntryObj->ItemIcon);
	NameTextBlock->SetText(EntryObj->ItemName);

	StatListView->ClearListItems();

	StatListView->SetListItems(EntryObj->StatEntryObjects);
}

FReply URewardEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Result = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (auto* PC = GetOwningPlayer<APlayerControllerBase>())
	{
		RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: Try get reward %s"), *LOG_LINE_INFO, *ItemRowName.ToString());
		PC->Server_RequestReward(ItemRowName);
	}

	return Result;
}

void URewardEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	FrameImage->SetBrushFromTexture(HoveringFrameTexture);
}

void URewardEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	FrameImage->SetBrushFromTexture(NormalFrameTexture);
}
