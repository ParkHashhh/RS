// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Core/RSUtils.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataSubsystem.generated.h"

struct FStageTableInfo;
struct FStageObjectiveTableInfo;
struct FMiscTableInfo;
struct FItemTableInfo;
struct FPlayerDefaultInfo;
struct FStatDisplayTableInfo;
struct FZombieDefaultInfo;
class UDataTable;
DECLARE_MULTICAST_DELEGATE(FOnTableInfoLoaded);

/**
 * 게임의 아이템 정보를 관리하는 서브시스템입니다.
 */
UCLASS(BlueprintType)
class RS_API UDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDataSubsystem();

private:
	inline static const FName DefaultItemRowName = TEXT("Default");

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ItemInfoTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> MiscInfoTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> PlayerDefaultInfoTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> StatDisplayInfoTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ZombieDefaultInfoTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> StageObjectiveInfoTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> StageInfoTable;

	FOnTableInfoLoaded OnItemInfoLoadedDelegate;
	FOnTableInfoLoaded OnMiscInfoLoadedDelegate;
	FOnTableInfoLoaded OnPlayerDefaultInfoLoadedDelegate;
	FOnTableInfoLoaded OnStatDisplayInfoLoadedDelegate;
	FOnTableInfoLoaded OnZombieDefaultInfoLoadedDelegate;
	FOnTableInfoLoaded OnStageObjectiveInfoLoadedDelegate;
	FOnTableInfoLoaded OnStageInfoLoadedDelegate;

	float TotalWeight{0.f};

public:
	template <typename T>
	void ExecuteOrBindOnItemInfoLoaded(T* Object, void (T::*Callback)())
	{
		RSUtils::ExecuteOrBind(IsItemTableLoaded(), OnItemInfoLoadedDelegate, Object, Callback);
	}

	template <typename T>
	void ExecuteOrBindOnMiscInfoLoaded(T* Object, void (T::*Callback)())
	{
		RSUtils::ExecuteOrBind(IsMiscTableLoaded(), OnMiscInfoLoadedDelegate, Object, Callback);
	}

	template <typename T>
	void ExecuteOrBindOnPlayerDefaultInfoLoaded(T* Object, void (T::*Callback)())
	{
		RSUtils::ExecuteOrBind(IsPlayerDefaultTableLoaded(), OnPlayerDefaultInfoLoadedDelegate, Object, Callback);
	}

	template <typename T>
	void ExecuteOrBindOnZombieDefaultInfoLoaded(T* Object, void (T::* Callback)())
	{
		RSUtils::ExecuteOrBind(IsZombieDefaultTableLoaded(), OnZombieDefaultInfoLoadedDelegate, Object, Callback);
	}

	template <typename T>
	void ExecuteOrBindOnStageObjectiveInfoLoaded(T* Object, void (T::* Callback)())
	{
		RSUtils::ExecuteOrBind(IsStageObjectiveInfoTableLoaded(), OnStageObjectiveInfoLoadedDelegate, Object, Callback);
	}

	template <typename T>
	void ExecuteOrBindOnStageInfoLoaded(T* Object, void (T::* Callback)())
	{
		RSUtils::ExecuteOrBind(IsStageInfoTableLoaded(), OnStageInfoLoadedDelegate, Object, Callback);
	}

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const FItemTableInfo* GetItemTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Item Table Info"))
	bool K2_GetItemTableInfo(FName Name, FItemTableInfo& OutItemInfo) const;

	FORCEINLINE bool IsItemTableLoaded() const
	{
		return IsValid(ItemInfoTable);
	}

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Item Table Loaded"))
	bool K2_IsItemTableLoaded() const
	{
		return IsItemTableLoaded();
	}

	const FMiscTableInfo* GetMiscTableInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Misc Table Info"))
	bool K2_GetMiscTableInfo(FMiscTableInfo& OutItemInfo) const;

	FORCEINLINE bool IsMiscTableLoaded() const { return IsValid(MiscInfoTable); }

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Misc Table Loaded"))
	bool K2_IsMiscTableLoaded() const { return IsMiscTableLoaded(); }

	const FPlayerDefaultInfo* GetPlayerDefaultTableInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Player Default Table Info"))
	bool K2_GetPlayerDefaultTableInfo(FPlayerDefaultInfo& OutPlayerDefaultInfo) const;

	FORCEINLINE bool IsPlayerDefaultTableLoaded() const
	{
		return IsValid(PlayerDefaultInfoTable);
	}

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Is Player Default Table Loaded"))
	bool K2_IsPlayerDefaultTableLoaded() const
	{
		return IsPlayerDefaultTableLoaded();
	}

	const FStatDisplayTableInfo* GetStatDisplayTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Stat Display Table Info"))
	bool K2_GetStatDisplayTableInfo(FName Name, FStatDisplayTableInfo& OutStatDisplayInfo) const;

	FORCEINLINE bool IsStatDisplayInfoTableLoaded() const
	{
		return IsValid(StatDisplayInfoTable);
	}

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Stat Display Info Table Loaded"))
	bool K2_IsStatDisplayInfoTableLoaded() const
	{
		return IsStatDisplayInfoTableLoaded();
	}

	const FStageObjectiveTableInfo* GetStageObjectiveTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Stage Objective Table Info"))
	bool K2_GetStageObjectiveTableInfo(FName Name, FStageObjectiveTableInfo& OutStageObjectiveInfo) const;

	bool IsStageObjectiveInfoTableLoaded() const
	{
		return IsValid(StageObjectiveInfoTable);
	}

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Stage Objective Info Table Loaded"))
	bool K2_IsStageObjectiveInfoTableLoaded() const
	{
		return IsStageObjectiveInfoTableLoaded();
	}

	const FStageTableInfo* GetStageTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Stage Table Info"))
	bool K2_GetStageTableInfo(FName Name, FStageTableInfo& OutStageInfo) const;

	bool IsStageInfoTableLoaded() const
	{
		return IsValid(StageInfoTable);
	}

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Stage Table Info Loaded"))
	bool K2_IsStageTableInfoLoaded() const
	{
		return IsStageInfoTableLoaded();
	}

	const UDataTable* GetItemInfoTable() const
	{
		return ItemInfoTable;
	}

	const UDataTable* GetMiscInfoTable() const
	{
		return MiscInfoTable;
	}

	const UDataTable* GetPlayerDefaultInfoTable() const
	{
		return PlayerDefaultInfoTable;
	}

	const UDataTable* GetStatDisplayInfoTable() const
	{
		return StatDisplayInfoTable;
	}

	const FZombieDefaultInfo* GetZombieDefaultTableInfo(const FName& RowName) const;

	FORCEINLINE bool IsZombieDefaultTableLoaded() const
	{
		return IsValid(ZombieDefaultInfoTable);
	}

	const UDataTable* GetStageObjectiveInfoTable() const
	{
		return StageObjectiveInfoTable;
	}

	const UDataTable* GetStageTableInfo() const
	{
		return StageInfoTable;
	}

	UFUNCTION(BlueprintPure)
	float GetTotalWeight() const
	{
		return TotalWeight;
	}

private:
	void LoadItemInfoTable();

	UFUNCTION()
	void OnItemInfoTableLoaded();

	void LoadMiscInfoTable();

	UFUNCTION()
	void OnMiscInfoTableLoaded();

	void LoadPlayerDefaultInfoTable();

	UFUNCTION()
	void OnPlayerDefaultInfoTableLoaded();

	void LoadStatDisplayInfoTable();

	UFUNCTION()
	void OnStatDisplayInfoTableLoaded();

	void ValidateItemInfoTable(const UDataTable* Table) const;

	void LoadZombieDefaultInfoTable();

	UFUNCTION()
	void OnZombieDefaultInfoTableLoaded();

	void LoadStageObjectiveInfoTable();

	UFUNCTION()
	void OnStageObjectiveInfoTableLoaded();

	void LoadStageInfoTable();

	UFUNCTION()
	void OnStageTableInfoLoaded();

#pragma region Item

public:
	TArray<FName> GetThreeRandomItmes(int32 CurrentStage) const;

private:
	TArray<FName> GetRandomItems(int32 Count, int32 CurrentStage) const;

#pragma endregion
};
