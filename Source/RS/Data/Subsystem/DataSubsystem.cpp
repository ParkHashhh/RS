// Fill out your copyright notice in the Description page of Project Settings.


#include "DataSubsystem.h"

#include "RS/Data/RSDataSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "RS/LogCategory.h"
#include "RS/RSTypes.h"
#include "RS/Data/DataAsset/ItemInfoTableAsset.h"
#include "RS/Data/DataAsset/MiscInfoTableAsset.h"
#include "RS/Data/DataAsset/PlayerDefaultInfoTableAsset.h"

void UDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadPlayerDefaultInfoTable();
	LoadItemInfoTable();
	LoadMiscInfoTable();
}

const FItemTableInfo* UDataSubsystem::GetItemTableInfo(FName Name) const
{
	if (!IsValid(ItemInfoTable))
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: ItemInfoTable is NOT valid when getting row %s. It might still be loading."), *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const auto* Row = ItemInfoTable->FindRow<FItemTableInfo>(Name, TEXT("ItemInfo"));
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

const FMiscTableInfo* UDataSubsystem::GetMiscTableInfo(FName Name) const
{
	if (!IsValid(MiscInfoTable))
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: MiscInfoTable is NOT valid when getting row %s. It might still be loading."), *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const auto* Row = ItemInfoTable->FindRow<FMiscTableInfo>(Name, TEXT("MiscInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in MiscInfoTable"), *LOG_LINE_INFO, *Name.ToString());
	}
	return Row;
}

bool UDataSubsystem::K2_GetMiscTableInfo(FName Name, FMiscTableInfo& OutItemInfo) const
{
	if (const FMiscTableInfo* ItemInfoPtr = GetMiscTableInfo(Name))
	{
		OutItemInfo = *ItemInfoPtr;
		return true;
	}

	OutItemInfo = FMiscTableInfo();
	return false;
}

const FPlayerDefaultInfo* UDataSubsystem::GetPlayerDefaultTableInfo(FName Name) const
{
	if (!IsValid(PlayerDefaultInfoTable))
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: PlayerDefaultInfoTable is NOT valid when getting row %s. It might still be loading."), *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const auto* Row = PlayerDefaultInfoTable->FindRow<FPlayerDefaultInfo>(Name, TEXT("PlayerDefaultInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in PlayerDefaultInfoTable"), *LOG_LINE_INFO, *Name.ToString());
	}
	return Row;
}

bool UDataSubsystem::K2_GetPlayerDefaultTableInfo(FName Name, FPlayerDefaultInfo& OutPlayerDefaultInfo) const
{
	if (const FPlayerDefaultInfo* PlayerDefaultInfoPtr = GetPlayerDefaultTableInfo(Name))
	{
		OutPlayerDefaultInfo = *PlayerDefaultInfoPtr;
		return true;
	}

	OutPlayerDefaultInfo = FPlayerDefaultInfo();
	return false;
}

void UDataSubsystem::LoadItemInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || !Settings->ItemInfoTableAsset.IsValid())
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: ItemInfoTableAssetId is invalid. Check Project Settings -> RS Data Settings."), *LOG_LINE_INFO);
		return;
	}

	const FPrimaryAssetId& TargetAssetId = Settings->ItemInfoTableAsset;

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

void UDataSubsystem::LoadMiscInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || !Settings->MiscInfoTableAsset.IsValid())
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: MiscInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."), *LOG_LINE_INFO);
		return;
	}

	const FPrimaryAssetId& TargetAssetId = Settings->MiscInfoTableAsset;

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load MiscInfoTableAsset (%s)"), *LOG_LINE_INFO, *TargetAssetId.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.LoadPrimaryAsset(TargetAssetId, TArray<FName>{},
								  FStreamableDelegate::CreateUObject(
									  this, &UDataSubsystem::OnMiscInfoTableLoaded, TargetAssetId));
}

void UDataSubsystem::OnMiscInfoTableLoaded(FPrimaryAssetId LoadedAssetID)
{
	const auto& AssetManager = UAssetManager::Get();
	const auto* DataAsset = AssetManager.GetPrimaryAssetObject<UMiscInfoTableAsset>(LoadedAssetID);
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get Primary Asset Object for %s"), *LOG_LINE_INFO, *LoadedAssetID.ToString());
		return;
	}

	MiscInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!MiscInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO, *LoadedAssetID.ToString());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: MiscInfoTable loaded successfully: %s"), *LOG_LINE_INFO, *MiscInfoTable->GetName());

	OnMiscInfoLoaded.Broadcast();
}

void UDataSubsystem::LoadPlayerDefaultInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || !Settings->PlayerDefaultInfoTableAsset.IsValid())
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: PlayerDefaultInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."), *LOG_LINE_INFO);
		return;
	}

	const FPrimaryAssetId& TargetAssetId = Settings->PlayerDefaultInfoTableAsset;

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load PlayerDefaultInfoTableAsset (%s)"), *LOG_LINE_INFO, *TargetAssetId.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.LoadPrimaryAsset(TargetAssetId, TArray<FName>{},
								  FStreamableDelegate::CreateUObject(
									  this, &UDataSubsystem::OnPlayerDefaultInfoTableLoaded, TargetAssetId));
}

void UDataSubsystem::OnPlayerDefaultInfoTableLoaded(FPrimaryAssetId LoadedAssetID)
{
	const auto& AssetManager = UAssetManager::Get();
	const auto* DataAsset = AssetManager.GetPrimaryAssetObject<UPlayerDefaultInfoTableAsset>(LoadedAssetID);
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get Primary Asset Object for %s"), *LOG_LINE_INFO, *LoadedAssetID.ToString());
		return;
	}

	PlayerDefaultInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!PlayerDefaultInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO, *LoadedAssetID.ToString());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: PlayerDefaultInfoTable loaded successfully: %s"), *LOG_LINE_INFO, *PlayerDefaultInfoTable->GetName());

	OnPlayerDefaultInfoLoaded.Broadcast();
}
