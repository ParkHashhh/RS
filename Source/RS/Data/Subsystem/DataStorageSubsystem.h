// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataStorageSubsystem.generated.h"

class ARSPlayerController;
class ARSPlayerState;
/**
 *
 */
UCLASS()
class RS_API UDataStorageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	static const FString SaveSlotName;

	TMap<FUniqueNetIdRepl, FName> PendingTextureRestore;
	TMap<FUniqueNetIdRepl, FName> PendingSkillRestore;
	// 임시로 인벤토리 정보를 저장. 간단하게 TableRowName과 Count를 저장
	TMap<FUniqueNetIdRepl, TArray<FItemStorageInfo>> PreviousInventoryStorage;

	// 전전스테이지 인벤토리. 직전 스테이지 인벤토리와 비교하여 직전스테이지에서'만' 신규로 획득한 아이템 판별 용
	TMap<FUniqueNetIdRepl, TArray<FItemStorageInfo>> PrePreviousInventoryStorage;

	TMap<FUniqueNetIdRepl, FStageResultStatInfo> PreviousStageResultStatStorage;

	TSet<FUniqueNetIdRepl> PendingStageResultPlayers;
public:
	void RestoreTextureSkill(ARSPlayerState* PS);
public:
	void StoreAllPlayerInventory();
	void RestorePlayerInventory(const ARSPlayerController* NewPlayer);
	void ClearPlayerInventory(const FUniqueNetIdRepl& UID);
	void SavePlayerDataToDisk(const ARSPlayerState* PS);
	bool LoadPlayerDataFromDisk(ARSPlayerState* PS);   // 로드 성공하면 true
	void DeletePlayerSaveFile();
	bool TryGetPrePreviousInventoryStorage(const APlayerState* InState, TArray<FItemStorageInfo>& OutStorage) const;

	void StoreAllPlayerStageResultStat();
	void RestorePlayerStageResultStat(const ARSPlayerController* NewPlayer);

public:
	void MarkPendingStageResult(const FUniqueNetIdRepl& UID);
	bool ConsumePendingStageResult(const FUniqueNetIdRepl& UID);

	bool HasPendingStageResult(const FUniqueNetIdRepl& UID) const;

#pragma region Character Select

private:
	UPROPERTY()
	FName SelectedTextureName;

	UPROPERTY()
	FName SelectedSkillName;

public:
	void SetSelectedCharacter(FName Texture, FName Skill);

	FName GetSelectedTextureName() const;

	FName GetSelectedSkillName() const;

#pragma endregion
};
