// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataSubsystem.generated.h"

struct FItemTableInfo;
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

public:
	UPROPERTY(BlueprintAssignable, Category = "Data")
	FOnItemInfoLoadedSignature OnItemInfoLoaded;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	[[nodiscard]] const FItemTableInfo* GetItemTableInfo(FName Name) const;

	UFUNCTION(BlueprintCallable, Category = "Item", meta=(DisplayName = "Get Item Table Info"))
	bool K2_GetItemTableInfo(FName Name, FItemTableInfo& OutItemInfo) const;

	/** 데이터 테이블이 성공적으로 로드되었는지 확인합니다. */
	UFUNCTION(BlueprintPure, Category = "Data")
	bool IsItemTableLoaded() const { return IsValid(ItemInfoTable); }


private:
	void LoadItemInfoTable();

	UFUNCTION()
	void OnItemInfoTableLoaded(FPrimaryAssetId LoadedAssetID);
};
