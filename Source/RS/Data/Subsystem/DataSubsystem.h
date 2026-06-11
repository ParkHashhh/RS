// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataSubsystem.generated.h"

struct FMiscTableInfo;
struct FItemTableInfo;
struct FPlayerDefaultInfo;
class UDataTable;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemInfoLoadedSignature);

/**
 * 게임의 아이템 정보를 관리하는 서브시스템입니다.
 */
UCLASS(BlueprintType)
class RS_API UDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	inline static const FName DefaultItemRowName = TEXT("Default");

	UPROPERTY()
	TObjectPtr<UDataTable> ItemInfoTable;

	UPROPERTY()
	TObjectPtr<UDataTable> MiscInfoTable;

	UPROPERTY()
	TObjectPtr<UDataTable> PlayerDefaultInfoTable;

public:
	UPROPERTY(BlueprintAssignable, Category = "Data")
	FOnItemInfoLoadedSignature OnItemInfoLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Data")
	FOnItemInfoLoadedSignature OnMiscInfoLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Data")
	FOnItemInfoLoadedSignature OnPlayerDefaultInfoLoaded;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const FItemTableInfo* GetItemTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Item Table Info"))
	bool K2_GetItemTableInfo(FName Name, FItemTableInfo& OutItemInfo) const;

	FORCEINLINE bool IsItemTableLoaded() const { return IsValid(ItemInfoTable); }

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Item Table Loaded"))
	bool K2_IsItemTableLoaded() const { return IsItemTableLoaded(); }

	const FMiscTableInfo* GetMiscTableInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Misc Table Info"))
	bool K2_GetMiscTableInfo(FMiscTableInfo& OutItemInfo) const;

	FORCEINLINE bool IsMiscTableLoaded() const { return IsValid(MiscInfoTable); }

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Misc Table Loaded"))
	bool K2_IsMiscTableLoaded() const { return IsMiscTableLoaded(); }

	const FPlayerDefaultInfo* GetPlayerDefaultTableInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Player Default Table Info"))
	bool K2_GetPlayerDefaultTableInfo(FPlayerDefaultInfo& OutPlayerDefaultInfo) const;

	FORCEINLINE bool IsPlayerDefaultTableLoaded() const { return IsValid(PlayerDefaultInfoTable); }

	UFUNCTION(BlueprintPure, Category = "Data", meta=(DisplayName = "Is Player Default Table Loaded"))
	bool K2_IsPlayerDefaultTableLoaded() const { return IsPlayerDefaultTableLoaded(); }

private:
	void LoadItemInfoTable();

	UFUNCTION()
	void OnItemInfoTableLoaded(FPrimaryAssetId LoadedAssetID);

	void LoadMiscInfoTable();

	UFUNCTION()
	void OnMiscInfoTableLoaded(FPrimaryAssetId LoadedAssetID);

	void LoadPlayerDefaultInfoTable();

	UFUNCTION()
	void OnPlayerDefaultInfoTableLoaded(FPrimaryAssetId LoadedAssetID);
};
