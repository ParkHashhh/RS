// Fill out your copyright notice in the Description page of Project Settings.


#include "DataSubsystem.h"

#include "Engine/AssetManager.h"
#include "RS/LogCategory.h"
#include "RS/RSTypes.h"
#include "RS/Data/DataAsset/ItemInfoTableAsset.h"

void UDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

const FItemTableInfo* UDataSubsystem::GetItemTableInfo(const FName Name) const
{
	if (!IsValid(ItemInfoTable))
	{
		return nullptr;
	}

	return ItemInfoTable->FindRow<FItemTableInfo>(Name, TEXT("ItemInfo"));
}

bool UDataSubsystem::K2_ShouldCreateSubsystem_Implementation(UObject* Outer) const
{
	return false;
}

void UDataSubsystem::LoadItemInfoTable()
{
	if (!ItemInfoTableAssetId.IsValid())
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: ItemInfoTableAssetId is invalid"), *LOG_LINE_INFO);
		return;
	}

	auto& AssetManager = UAssetManager::Get();
	AssetManager.LoadPrimaryAsset(ItemInfoTableAssetId, TArray<FName>{},
	                              FStreamableDelegate::CreateUObject(
		                              this, &UDataSubsystem::OnItemInfoTableLoaded, ItemInfoTableAssetId));
}

void UDataSubsystem::OnItemInfoTableLoaded(FPrimaryAssetId LoadedAssetID)
{
	const auto& AssetManager = UAssetManager::Get();
	const auto* DataAsset = AssetManager.GetPrimaryAssetObject<UItemInfoTableAsset>(LoadedAssetID);
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: Failed to load ItemInfoTableAsset"), *LOG_LINE_INFO);
		return;
	}

	ItemInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!ItemInfoTable)
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: Failed to load ItemInfoTable"), *LOG_LINE_INFO);
		return;
	}

	RS_LOG_DEBUG(LogRSProperties, Log, TEXT("%s: ItemInfoTable loaded successfully"), *LOG_LINE_INFO);

	if (OnItemInfoLoadedDelegate.IsBound())
	{
		OnItemInfoLoadedDelegate.Broadcast();
	}
}
