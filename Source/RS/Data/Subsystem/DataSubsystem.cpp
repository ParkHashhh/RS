// Fill out your copyright notice in the Description page of Project Settings.


#include "DataSubsystem.h"

#include "RS/Data/RSDataSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "RS/LogCategory.h"
#include "RS/RSTypes.h"
#include "RS/Data/DataAsset/ItemInfoTableAsset.h"

void UDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadItemInfoTable();
}

const FItemTableInfo* UDataSubsystem::GetItemTableInfo(FName Name) const
{
	if (!IsValid(ItemInfoTable))
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: ItemInfoTable is NOT valid when getting row %s. It might still be loading."), *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const FItemTableInfo* Row = ItemInfoTable->FindRow<FItemTableInfo>(Name, TEXT("ItemInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in ItemInfoTable"), *LOG_LINE_INFO, *Name.ToString());
	}
	return Row;
}

bool UDataSubsystem::K2_GetItemTableInfo(FName Name, FItemTableInfo& OutItemInfo) const
{
	if (const FItemTableInfo* ItemInfoPtr = GetItemTableInfo(Name))
	{
		OutItemInfo = *ItemInfoPtr;
		return true;
	}

	OutItemInfo = FItemTableInfo();
	return false;
}

void UDataSubsystem::LoadItemInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || !Settings->ItemInfoTableAssetId.IsValid())
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: ItemInfoTableAssetId is invalid. Check Project Settings -> RS Data Settings."), *LOG_LINE_INFO);
		return;
	}

	const FPrimaryAssetId& TargetAssetId = Settings->ItemInfoTableAssetId;

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load ItemInfoTableAsset (%s)"), *LOG_LINE_INFO, *TargetAssetId.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.LoadPrimaryAsset(TargetAssetId, TArray<FName>{},
	                              FStreamableDelegate::CreateUObject(
		                              this, &UDataSubsystem::OnItemInfoTableLoaded, TargetAssetId));
}

void UDataSubsystem::OnItemInfoTableLoaded(FPrimaryAssetId LoadedAssetID)
{
	const auto& AssetManager = UAssetManager::Get();
	const auto* DataAsset = AssetManager.GetPrimaryAssetObject<UItemInfoTableAsset>(LoadedAssetID);
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get Primary Asset Object for %s"), *LOG_LINE_INFO, *LoadedAssetID.ToString());
		return;
	}

	ItemInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!ItemInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO, *LoadedAssetID.ToString());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: ItemInfoTable loaded successfully: %s"), *LOG_LINE_INFO, *ItemInfoTable->GetName());

	OnItemInfoLoaded.Broadcast();
}
