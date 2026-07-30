// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataStorageSubsystem.generated.h"

class ARSPlayerController;

USTRUCT()
struct RS_API FItemStorageInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FName RowName;

	UPROPERTY()
	int32 Count;
};

USTRUCT()
struct RS_API FStageResultInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int32 KillCount;

	UPROPERTY()
	TArray<FItemStorageInfo> AchievedItems;
};

/**
 *
 */
UCLASS()
class RS_API UDataStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	// 임시로 인벤토리 정보를 저장. 간단하게 TableRowName과 Count를 저장
	TMap<FUniqueNetIdRepl, TArray<FItemStorageInfo>> TempInventoryStorage;

	// 플레이어의 이전 스테이지 통계. 현재는 새로 획득한 아이템과 전체 킬수만 보존. AMainGameMode에서 집계하는 것 중 필요한 것만
	TMap<FUniqueNetIdRepl, FStageResultInfo> StageResultStorage;

public:
	void StoreAllPlayerInventory();
	void RestorePlayerInventory(const ARSPlayerController* NewPlayer);

	void StoreAllPlayerStageResult();
	void RestorePlayerStageResult(const ARSPlayerController* NewPlayer);
};
