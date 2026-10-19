// Fill out your copyright notice in the Description page of Project Settings.


#include "StatEntryObject.h"

#include "RS/Core/LogCategory.h"

void UStatEntryObject::Initialize(const FText& InStatName, const FText& InStatDescription, UPaperSprite* InIcon)
{
	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: StatEntryObject initialized"), *LOG_LINE_INFO);

	StatName = InStatName;
	StatDescription = InStatDescription;
	Icon = InIcon;
}
