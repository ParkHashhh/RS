// Fill out your copyright notice in the Description page of Project Settings.


#include "DataSubsystem.h"

#include "GameplayEffect.h"
#include "RS/Data/RSDataSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "RS/LogCategory.h"
#include "RS/RSTypes.h"
#include "RS/Data/DataAsset/ItemInfoTableAsset.h"
#include "RS/Data/DataAsset/MiscInfoTableAsset.h"
#include "RS/Data/DataAsset/PlayerDefaultInfoTableAsset.h"
#include "RS/Data/DataAsset/StageInfoTableAsset.h"
#include "RS/Data/DataAsset/StageObjectiveInfoTableAsset.h"
#include "RS/Data/DataAsset/StatDisplayInfoTableAsset.h"
#include "RS/Data/DataAsset/ZombieDefaultInfoTableAsset.h"

UDataSubsystem::UDataSubsystem()
{
}

void UDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadPlayerDefaultInfoTable();
	LoadItemInfoTable();
	LoadMiscInfoTable();
	LoadStatDisplayInfoTable();
	LoadZombieDefaultInfoTable();
	LoadStageObjectiveInfoTable();
	LoadStageInfoTable();
}

const FItemTableInfo* UDataSubsystem::GetItemTableInfo(FName Name) const
{
	if (!ItemInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Warning,
		             TEXT("%s: ItemInfoTable is NOT valid when getting row %s. It might still be loading."),
		             *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const auto* Row = ItemInfoTable->FindRow<FItemTableInfo>(Name, TEXT("ItemInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in ItemInfoTable"), *LOG_LINE_INFO,
		             *Name.ToString());
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

const FMiscTableInfo* UDataSubsystem::GetMiscTableInfo() const
{
	if (!MiscInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Warning,
		             TEXT("%s: MiscInfoTable is NOT valid when getting row %s. It might still be loading."),
		             *LOG_LINE_INFO, *DefaultItemRowName.ToString());
		return nullptr;
	}

	const auto* Row = MiscInfoTable->FindRow<FMiscTableInfo>(DefaultItemRowName, TEXT("MiscInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in MiscInfoTable"), *LOG_LINE_INFO,
		             *DefaultItemRowName.ToString());
	}

	return Row;
}

bool UDataSubsystem::K2_GetMiscTableInfo(FMiscTableInfo& OutItemInfo) const
{
	if (const FMiscTableInfo* ItemInfoPtr = GetMiscTableInfo())
	{
		OutItemInfo = *ItemInfoPtr;
		return true;
	}

	OutItemInfo = FMiscTableInfo();
	return false;
}

const FPlayerDefaultInfo* UDataSubsystem::GetPlayerDefaultTableInfo() const
{
	if (!PlayerDefaultInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Warning,
		             TEXT("%s: PlayerDefaultInfoTable is NOT valid when getting row %s. It might still be loading."),
		             *LOG_LINE_INFO, *DefaultItemRowName.ToString());
		return nullptr;
	}

	const auto* Row = PlayerDefaultInfoTable->FindRow<
		FPlayerDefaultInfo>(DefaultItemRowName, TEXT("PlayerDefaultInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in PlayerDefaultInfoTable"), *LOG_LINE_INFO,
		             *DefaultItemRowName.ToString());
	}

	return Row;
}

const FZombieDefaultInfo* UDataSubsystem::GetZombieDefaultTableInfo(const FName& RowName) const
{
	if (!ZombieDefaultInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Warning,
		             TEXT("%s: ZombieDefaultInfoTable is NOT valid when getting row %s. It might still be loading."),
		             *LOG_LINE_INFO, *DefaultItemRowName.ToString());
		return nullptr;
	}

	const auto* Row = ZombieDefaultInfoTable->FindRow<FZombieDefaultInfo>(RowName, TEXT("ZombieDefaultInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in ZombieDefaultInfoTable"), *LOG_LINE_INFO,
		             *RowName.ToString());
	}

	return Row;
}

bool UDataSubsystem::K2_GetPlayerDefaultTableInfo(FPlayerDefaultInfo& OutPlayerDefaultInfo) const
{
	if (const FPlayerDefaultInfo* PlayerDefaultInfoPtr = GetPlayerDefaultTableInfo())
	{
		OutPlayerDefaultInfo = *PlayerDefaultInfoPtr;
		return true;
	}

	OutPlayerDefaultInfo = FPlayerDefaultInfo();
	return false;
}

const FStatDisplayTableInfo* UDataSubsystem::GetStatDisplayTableInfo(FName Name) const
{
	if (!StatDisplayInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Warning,
		             TEXT("%s: StatDisplayInfoTable is NOT valid when getting row %s. It might still be loading."),
		             *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const auto* Row = StatDisplayInfoTable->FindRow<FStatDisplayTableInfo>(Name, TEXT("StatDisplayInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in StatDisplayInfoTable"), *LOG_LINE_INFO,
		             *Name.ToString());
	}

	return Row;
}

bool UDataSubsystem::K2_GetStatDisplayTableInfo(FName Name, FStatDisplayTableInfo& OutStatDisplayInfo) const
{
	if (const FStatDisplayTableInfo* StatDisplayInfo = GetStatDisplayTableInfo(Name))
	{
		OutStatDisplayInfo = *StatDisplayInfo;
		return true;
	}

	OutStatDisplayInfo = FStatDisplayTableInfo();
	return false;
}

const FStageObjectiveTableInfo* UDataSubsystem::GetStageObjectiveTableInfo(FName Name) const
{
	if (!StageObjectiveInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Warning,
		             TEXT("%s: StageObjectiveInfoTable is NOT valid when getting row %s. It might still be loading."),
		             *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const auto* Row = StageObjectiveInfoTable->FindRow<FStageObjectiveTableInfo>(Name, TEXT("StageObjectiveInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in StageObjectiveInfoTable"), *LOG_LINE_INFO,
		             *Name.ToString());
	}

	return Row;
}

bool UDataSubsystem::K2_GetStageObjectiveTableInfo(FName Name, FStageObjectiveTableInfo& OutStageObjectiveInfo) const
{
	if (const FStageObjectiveTableInfo* Info = GetStageObjectiveTableInfo(Name))
	{
		OutStageObjectiveInfo = *Info;
		return true;
	}

	OutStageObjectiveInfo = FStageObjectiveTableInfo();
	return false;
}

const FStageTableInfo* UDataSubsystem::GetStageTableInfo(FName Name) const
{
	if (!StageInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Warning,
		             TEXT("%s: StageInfoTable is NOT valid when getting row %s. It might still be loading."),
		             *LOG_LINE_INFO, *Name.ToString());
		return nullptr;
	}

	const auto* Row = StageInfoTable->FindRow<FStageTableInfo>(Name, TEXT("StageInfo"));
	if (!Row)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Failed to find row %s in StageInfoTable"), *LOG_LINE_INFO,
		             *Name.ToString());
	}

	return Row;
}

bool UDataSubsystem::K2_GetStageTableInfo(FName Name, FStageTableInfo& OutStageInfo) const
{
	if (const FStageTableInfo* Info = GetStageTableInfo(Name))
	{
		OutStageInfo = *Info;
		return true;
	}

	OutStageInfo = FStageTableInfo();
	return false;
}

void UDataSubsystem::LoadItemInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || Settings->ItemInfoTableAsset.IsNull())
	{
		RS_LOG_DEBUG(LogRSData, Error,
		             TEXT("%s: ItemInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."),
		             *LOG_LINE_INFO);
		return;
	}

	const FSoftObjectPath& TargetPath = Settings->ItemInfoTableAsset.ToSoftObjectPath();

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load ItemInfoTableAsset (%s)"), *LOG_LINE_INFO,
	             *TargetPath.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.GetStreamableManager().RequestAsyncLoad(TargetPath,
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UDataSubsystem::OnItemInfoTableLoaded));
}

void UDataSubsystem::OnItemInfoTableLoaded()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const auto* DataAsset = Settings->ItemInfoTableAsset.Get();
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get DataAsset for ItemInfoTableAsset"), *LOG_LINE_INFO);
		return;
	}

	ItemInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!ItemInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO,
		             *DataAsset->GetName());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: ItemInfoTable loaded successfully: %s"), *LOG_LINE_INFO,
	             *ItemInfoTable->GetName());

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	ValidateItemInfoTable(ItemInfoTable);
#endif

	TArray<FItemTableInfo*> ItemInfos;
	ItemInfoTable->GetAllRows(TEXT("OnItemInfoTableLoaded"), ItemInfos);

	TotalWeight = 0.0f;
	for (const FItemTableInfo* Row : ItemInfos)
	{
		if (Row && Row->Weight > 0.0f)
		{
			TotalWeight += Row->Weight;
		}
	}

	OnItemInfoLoadedDelegate.Broadcast();
}

void UDataSubsystem::LoadMiscInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || Settings->MiscInfoTableAsset.IsNull())
	{
		RS_LOG_DEBUG(LogRSData, Error,
		             TEXT("%s: MiscInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."),
		             *LOG_LINE_INFO);
		return;
	}

	const FSoftObjectPath& TargetPath = Settings->MiscInfoTableAsset.ToSoftObjectPath();

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load MiscInfoTableAsset (%s)"), *LOG_LINE_INFO,
	             *TargetPath.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.GetStreamableManager().RequestAsyncLoad(TargetPath,
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UDataSubsystem::OnMiscInfoTableLoaded));
}

void UDataSubsystem::OnMiscInfoTableLoaded()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const auto* DataAsset = Settings->MiscInfoTableAsset.Get();
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get DataAsset for MiscInfoTableAsset"), *LOG_LINE_INFO);
		return;
	}

	MiscInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!MiscInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO,
		             *DataAsset->GetName());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: MiscInfoTable loaded successfully: %s"), *LOG_LINE_INFO,
	             *MiscInfoTable->GetName());

	OnMiscInfoLoadedDelegate.Broadcast();
}

void UDataSubsystem::LoadPlayerDefaultInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || Settings->PlayerDefaultInfoTableAsset.IsNull())
	{
		RS_LOG_DEBUG(LogRSData, Error,
		             TEXT("%s: PlayerDefaultInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."),
		             *LOG_LINE_INFO);
		return;
	}

	const FSoftObjectPath& TargetPath = Settings->PlayerDefaultInfoTableAsset.ToSoftObjectPath();

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load PlayerDefaultInfoTableAsset (%s)"), *LOG_LINE_INFO,
	             *TargetPath.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.GetStreamableManager().RequestAsyncLoad(TargetPath,
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UDataSubsystem::OnPlayerDefaultInfoTableLoaded));
}

void UDataSubsystem::OnPlayerDefaultInfoTableLoaded()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const auto* DataAsset = Settings->PlayerDefaultInfoTableAsset.Get();
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get DataAsset for PlayerDefaultInfoTableAsset"),
		             *LOG_LINE_INFO);
		return;
	}

	PlayerDefaultInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!PlayerDefaultInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO,
		             *DataAsset->GetName());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: PlayerDefaultInfoTable loaded successfully: %s"), *LOG_LINE_INFO,
	             *PlayerDefaultInfoTable->GetName());

	OnPlayerDefaultInfoLoadedDelegate.Broadcast();
}

void UDataSubsystem::LoadStatDisplayInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || Settings->StatDisplayInfoTableAsset.IsNull())
	{
		RS_LOG_DEBUG(LogRSData, Error,
		             TEXT("%s: StatDisplayInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."),
		             *LOG_LINE_INFO);
		return;
	}

	const FSoftObjectPath& TargetPath = Settings->StatDisplayInfoTableAsset.ToSoftObjectPath();

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load StatDisplayInfoTableAsset (%s)"), *LOG_LINE_INFO,
	             *TargetPath.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.GetStreamableManager().RequestAsyncLoad(TargetPath,
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UDataSubsystem::OnStatDisplayInfoTableLoaded));
}

void UDataSubsystem::OnStatDisplayInfoTableLoaded()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const auto* DataAsset = Settings->StatDisplayInfoTableAsset.Get();
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get DataAsset for StatDisplayInfoTableAsset"),
		             *LOG_LINE_INFO);
		return;
	}

	StatDisplayInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!StatDisplayInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO,
		             *DataAsset->GetName());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: StatDisplayInfoTable loaded successfully: %s"), *LOG_LINE_INFO,
	             *StatDisplayInfoTable->GetName());


	OnStatDisplayInfoLoadedDelegate.Broadcast();
}

void UDataSubsystem::ValidateItemInfoTable(const UDataTable* Table) const
{
	TArray<FItemTableInfo*> Rows;
	Table->GetAllRows(TEXT("ValidateItemInfoTable"), Rows);

	for (const FItemTableInfo* Row : Rows)
	{
		TSubclassOf<UGameplayEffect> EffectClass = Row->MasterItemEffectClass;
		const UGameplayEffect* GE = EffectClass->GetDefaultObject<UGameplayEffect>();
		ensureMsgf(GE, TEXT("%s: Invalid GameplayEffect class: %s"), *LOG_LINE_INFO, *EffectClass->GetName());

		for (auto& Modifier : GE->Modifiers)
		{
			if (Modifier.ModifierMagnitude.GetMagnitudeCalculationType() !=
				EGameplayEffectMagnitudeCalculation::SetByCaller)
			{
				RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: %s must use SetByCaller"), *LOG_LINE_INFO,
				             *EffectClass->GetName());

				ensureMsgf(false, TEXT("%s: %s must use SetByCaller"), *LOG_LINE_INFO, *EffectClass->GetName());
			}
			else if (!Modifier.ModifierMagnitude.GetSetByCallerFloat().DataTag.IsValid())
			{
				RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: %s must have a valid DataTag"), *LOG_LINE_INFO,
				             *EffectClass->GetName());

				ensureMsgf(false, TEXT("%s: %s must have a valid DataTag"), *LOG_LINE_INFO, *EffectClass->GetName());
			}

			// 기타 검사 로직 추가.
		}
	}
}

void UDataSubsystem::LoadZombieDefaultInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || Settings->ZombieDefaultInfoTableAsset.IsNull())
	{
		RS_LOG_DEBUG(LogRSData, Error,
		             TEXT("%s: ZombieDefaultInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."),
		             *LOG_LINE_INFO);
		return;
	}

	const FSoftObjectPath& TargetPath = Settings->ZombieDefaultInfoTableAsset.ToSoftObjectPath();

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load ZombieDefaultInfoTableAsset (%s)"), *LOG_LINE_INFO,
	             *TargetPath.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.GetStreamableManager().RequestAsyncLoad(TargetPath,
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UDataSubsystem::OnZombieDefaultInfoTableLoaded));
}

void UDataSubsystem::OnZombieDefaultInfoTableLoaded()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const auto* DataAsset = Settings->ZombieDefaultInfoTableAsset.Get();
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get DataAsset for ZombieDefaultInfoTableAsset"),
		             *LOG_LINE_INFO);
		return;
	}

	ZombieDefaultInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!ZombieDefaultInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO,
		             *DataAsset->GetName());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: ZombieDefaultInfoTable loaded successfully: %s"), *LOG_LINE_INFO,
	             *ZombieDefaultInfoTable->GetName());

	OnZombieDefaultInfoLoadedDelegate.Broadcast();
}

void UDataSubsystem::LoadStageObjectiveInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || Settings->StageObjectiveInfoTableAsset.IsNull())
	{
		RS_LOG_DEBUG(LogRSData, Error,
		             TEXT("%s: StageObjectiveInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."),
		             *LOG_LINE_INFO);
		return;
	}

	const FSoftObjectPath& TargetPath = Settings->StageObjectiveInfoTableAsset.ToSoftObjectPath();

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load StageObjectiveInfoTableAsset (%s)"), *LOG_LINE_INFO,
	             *TargetPath.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.GetStreamableManager().RequestAsyncLoad(TargetPath,
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UDataSubsystem::OnStageObjectiveInfoTableLoaded));
}

void UDataSubsystem::OnStageObjectiveInfoTableLoaded()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const auto* DataAsset = Settings->StageObjectiveInfoTableAsset.Get();
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get DataAsset for StageObjectiveInfoTableAsset"),
		             *LOG_LINE_INFO);
		return;
	}

	StageObjectiveInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!StageObjectiveInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO,
		             *DataAsset->GetName());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: StageObjectiveInfoTable loaded successfully: %s"), *LOG_LINE_INFO,
	             *StageObjectiveInfoTable->GetName());

	OnStageObjectiveInfoLoadedDelegate.Broadcast();
}

void UDataSubsystem::LoadStageInfoTable()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	if (!Settings || Settings->StageInfoTableAsset.IsNull())
	{
		RS_LOG_DEBUG(LogRSData, Error,
		             TEXT("%s: StageInfoTableAsset is invalid. Check Project Settings -> RS Data Settings."),
		             *LOG_LINE_INFO);
		return;
	}

	const FSoftObjectPath& TargetPath = Settings->StageInfoTableAsset.ToSoftObjectPath();

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: Starting to load StageInfoTableAsset (%s)"), *LOG_LINE_INFO,
	             *TargetPath.ToString());

	auto& AssetManager = UAssetManager::Get();
	AssetManager.GetStreamableManager().RequestAsyncLoad(TargetPath,
	                                                     FStreamableDelegate::CreateUObject(
		                                                     this, &UDataSubsystem::OnStageTableInfoLoaded));
}

void UDataSubsystem::OnStageTableInfoLoaded()
{
	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const auto* DataAsset = Settings->StageInfoTableAsset.Get();
	if (!IsValid(DataAsset))
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to get DataAsset for StageInfoTableAsset"), *LOG_LINE_INFO);
		return;
	}

	StageInfoTable = DataAsset->DataTable.LoadSynchronous();
	if (!StageInfoTable)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Failed to load DataTable from DataAsset %s"), *LOG_LINE_INFO,
		             *DataAsset->GetName());
		return;
	}

	RS_LOG_DEBUG(LogRSData, Log, TEXT("%s: StageInfoTable loaded successfully: %s"), *LOG_LINE_INFO,
	             *StageInfoTable->GetName());

	OnStageInfoLoadedDelegate.Broadcast();
}

TArray<FName> UDataSubsystem::GetThreeRandomItmes() const
{
	return GetRandomItems(3);
}

TArray<FName> UDataSubsystem::GetRandomItems(int32 Count) const
{
	TArray<FName> Result;

	if (!IsItemTableLoaded() || Count <= 0)
	{
		return Result;
	}

	struct FCandidate
	{
		FName RowName;
		const FItemTableInfo* Info;
	};

	TArray<FCandidate> Candidates;

	for (const auto& [Key, Value] : ItemInfoTable->GetRowMap())
	{
		const auto* Info = reinterpret_cast<FItemTableInfo*>(Value);
		if (Info && Info->Weight > 0.0f)
		{
			Candidates.Emplace(Key, Info);
		}
	}

	Count = FMath::Min(Count, Candidates.Num());

	float TempTotalWeight = TotalWeight;
	while (Result.Num() < Count && !Candidates.IsEmpty())
	{
		const float Dice = FMath::FRandRange(0.0f, TotalWeight);

		float CurrentWeight = 0.0f;
		for (int32 i = 0; i < Candidates.Num(); ++i)
		{
			auto& [RowName, Info] = Candidates[i];
			CurrentWeight += Info->Weight;
			if (Dice <= CurrentWeight)
			{
				Result.Emplace(RowName);
				Candidates.RemoveAtSwap(i);
				TempTotalWeight -= Info->Weight;
				break;
			}
		}
	}

	return Result;
}
