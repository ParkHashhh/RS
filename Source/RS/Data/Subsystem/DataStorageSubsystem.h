// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSTypes.h"
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

	// 임시로 인벤토리 정보를 저장. 간단하게 TableRowName과 Count를 저장
	TMap<FUniqueNetIdRepl, TArray<FItemStorageInfo>> PreviousInventoryStorage;

	// 전전스테이지 인벤토리. 직전 스테이지 인벤토리와 비교하여 직전스테이지에서'만' 신규로 획득한 아이템 판별 용
	TMap<FUniqueNetIdRepl, TArray<FItemStorageInfo>> PrePreviousInventoryStorage;

	TMap<FUniqueNetIdRepl, FStageResultStatInfo> PreviousStageResultStatStorage;

	TSet<FUniqueNetIdRepl> PendingStageResultPlayers;

public:
	void StoreAllPlayerInventory();
	void RestorePlayerInventory(const ARSPlayerController* NewPlayer);
	void ClearPlayerInventory(const FUniqueNetIdRepl& UID);
	bool TryGetPrePreviousInventoryStorage(const APlayerState* InState, TArray<FItemStorageInfo>& OutStorage) const;

	void StoreAllPlayerStageResultStat();
	void RestorePlayerStageResultStat(const ARSPlayerController* NewPlayer);

public:
	void MarkPendingStageResult(const FUniqueNetIdRepl& UID);
	bool ConsumePendingStageResult(const FUniqueNetIdRepl& UID);

	bool HasPendingStageResult(const FUniqueNetIdRepl& UID) const;
};
