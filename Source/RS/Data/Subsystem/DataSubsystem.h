// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataSubsystem.generated.h"

struct FItemTableInfo;
DECLARE_MULTICAST_DELEGATE(FOnTableLoaded)

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class RS_API UDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	FPrimaryAssetId ItemInfoTableAssetId;

	UPROPERTY()
	TObjectPtr<UDataTable> ItemInfoTable;

public:
	FOnTableLoaded OnItemInfoLoadedDelegate;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	[[nodiscard]] const FItemTableInfo* GetItemTableInfo(const FName Name) const;

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override
	{
		return K2_ShouldCreateSubsystem(Outer);
	}

protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Subsystem")
	bool K2_ShouldCreateSubsystem(UObject* Outer) const;
	virtual bool K2_ShouldCreateSubsystem_Implementation(UObject* Outer) const;

private:
	void LoadItemInfoTable();

	UFUNCTION()
	void OnItemInfoTableLoaded(FPrimaryAssetId LoadedAssetID);
};
