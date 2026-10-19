// Fill out your copyright notice in the Description page of Project Settings.


#include "RewardEntryWidget.h"

#include "RewardEntryObject.h"
#include "Components/Image.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "RS/UI/Battle/Inventory/Popup/Components/StatEntryObject.h"
#include "PaperSprite.h"
#include "RS/Core/LogCategory.h"
#include "RS/Core/RSUtils.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/Controller/PlayerControllerBase.h"

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

	// 마우스 호버 이벤트에서만 세팅되던 걸 최초 표시 시점에도 넣어줌 - 안 그러면 한 번도 안 건드린 카드는 프레임이 비어있음
	FrameImage->SetBrushFromTexture(NormalFrameTexture);

	if (const UGameInstance* GI = GetGameInstance())
	{
		FrameImage->SetColorAndOpacity(RSUtils::GetGradeColor(EntryObj->Grade, GI->GetSubsystem<UDataSubsystem>()));
		IconBorderImage->SetColorAndOpacity(RSUtils::GetGradeColor(EntryObj->Grade, GI->GetSubsystem<UDataSubsystem>()));
	}

	StatListView->ClearListItems();

	StatListView->SetListItems(EntryObj->StatEntryObjects);
}

FReply URewardEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Result = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (auto* PC = GetOwningPlayer<APlayerControllerBase>())
	{
		RS_LOG_DEBUG(LogRSInteraction, Warning, TEXT("%s: Try get reward %s"), *LOG_LINE_INFO, *ItemRowName.ToString());
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
