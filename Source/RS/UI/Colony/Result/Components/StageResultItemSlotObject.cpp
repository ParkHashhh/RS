// Fill out your copyright notice in the Description page of Project Settings.


#include "StageResultItemSlotObject.h"

#include "RS/RSTypes.h"

void UStageResultItemSlotObject::Initialize(const FItemStorageInfo& InItem)
{
	ItemRowName = InItem.RowName;
	ItemCount = InItem.Count;
}
