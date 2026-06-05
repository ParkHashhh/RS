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

	[[nodiscard]] const FItemTableInfo* GetItemTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Item Table Info"))
	bool K2_GetItemTableInfo(FName Name, FItemTableInfo& OutItemInfo) const;

	UFUNCTION(BlueprintPure, Category = "Data")
	bool IsItemTableLoaded() const { return IsValid(ItemInfoTable); }

	[[nodiscard]] const FMiscTableInfo* GetMiscTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Misc Table Info"))
	bool K2_GetMiscTableInfo(FName Name, FMiscTableInfo& OutItemInfo) const;

	UFUNCTION(BlueprintPure, Category = "Data")
	bool IsMiscTableLoaded() const { return IsValid(MiscInfoTable); }

	[[nodiscard]] const FPlayerDefaultInfo* GetPlayerDefaultTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Data", meta=(DisplayName = "Get Player Default Table Info"))
	bool K2_GetPlayerDefaultTableInfo(FName Name, FPlayerDefaultInfo& OutPlayerDefaultInfo) const;

	UFUNCTION(BlueprintPure, Category = "Data")
	bool IsPlayerDefaultTableLoaded() const { return IsValid(PlayerDefaultInfoTable); }

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
