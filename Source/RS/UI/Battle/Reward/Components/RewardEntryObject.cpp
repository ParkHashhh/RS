// Fill out your copyright notice in the Description page of Project Settings.


#include "RewardEntryObject.h"

#include "RS/Core/RSTypes.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/UI/Battle/Inventory/Popup/Components/StatEntryObject.h"

void URewardEntryObject::Initialize(FName InItemRowName, const UDataSubsystem* DataSubsystem)
{
	const FItemTableInfo* ItemInfo = DataSubsystem->GetItemTableInfo(InItemRowName);
	if (!ItemInfo)
	{
		return;
	}

	ItemRowName = InItemRowName;
	ItemIcon = ItemInfo->IconSprite;
	ItemName = ItemInfo->Name;
	Grade = ItemInfo->Grade;

	StatEntryObjects.Reset(ItemInfo->ItemOptions.Num());
	for (const FItemOption& ItemOption : ItemInfo->ItemOptions)
	{
		const FName TagName = ItemOption.OptionTag.GetTagName();
		if (const FStatDisplayTableInfo* DisplayInfo = DataSubsystem->GetStatDisplayTableInfo(TagName))
		{
			auto* NewObj = NewObject<UStatEntryObject>(this);
			NewObj->Initialize(DisplayInfo->DisplayName,
			                   RSUtils::GetStatDescription(*DisplayInfo, {ItemOption.Value}), DisplayInfo->Icon);

			StatEntryObjects.Emplace(NewObj);
		}
	}
}
