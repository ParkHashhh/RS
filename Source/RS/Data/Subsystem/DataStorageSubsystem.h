// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataStorageSubsystem.generated.h"

class ARSPlayerController;
/**
 *
 */
UCLASS()
class RS_API UDataStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// 임시로 인벤토리 정보를 저장. 간단하게 TableRowName과 Count를 저장. 이후 Guid 등 상세한 정보를 저장하려면 구조체 필요
	TMap<FUniqueNetIdRepl, TArray<TTuple<FName, int32>>> TempInventoryStorage;

public:
	void StoreAllPlayerInventory();

	void RestorePlayerInventory(const ARSPlayerController* NewPlayer);
};
