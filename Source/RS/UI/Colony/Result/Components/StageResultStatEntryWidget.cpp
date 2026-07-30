// Fill out your copyright notice in the Description page of Project Settings.


#include "StageResultStatEntryWidget.h"

#include "StageResultStatEntryObject.h"
#include "Components/TextBlock.h"

void UStageResultStatEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const auto* EntryObj = Cast<UStageResultStatEntryObject>(ListItemObject);
	if (!EntryObj)
	{
		return;
	}

	StatNameText->SetText(EntryObj->GetStatNameText());
	StatValueText->SetText(EntryObj->GetStatValueText());
}
