// Fill out your copyright notice in the Description page of Project Settings.


#include "StatEntryWidget.h"

#include "StatEntryObject.h"
#include "Components/TextBlock.h"
#include "RS/LogCategory.h"

void UStatEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const auto* StatObj = Cast<UStatEntryObject>(ListItemObject);
	if (!StatObj)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to cast ListItemObject to UStatEntryObject"), *LOG_LINE_INFO);
		return;
	}

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: StatEntryWidget initialized with StatName: %s"), *LOG_LINE_INFO, *StatObj->StatName.ToString());

	NameText->SetText(StatObj->StatName);
	DescriptionText->SetText(StatObj->StatDescription);
}
