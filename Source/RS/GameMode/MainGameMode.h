// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RSInGameMode.h"
#include "RS/RSTypes.h"
#include "RS/Utils/KillableInterface.h"
#include "MainGameMode.generated.h"

class AZombieBase;
class UZombieManager;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnKillInfoChanged, const FKillInfo*)

/**
 *
 */
UCLASS()
class RS_API AMainGameMode : public ARSInGameMode
{
	GENERATED_BODY()

#pragma region Common
private:
	UPROPERTY()
	TArray<FKillInfo> KillInfos;

	UPROPERTY()
	float StageElapsedTime = 0.0f;

	UPROPERTY()
	bool bStartStage = false;

	FOnKillInfoChanged OnKillInfoChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UZombieManager> StageZombieManagerComponent;

public:
	AMainGameMode();

	virtual void Tick(float DeltaSeconds) override;

	virtual void StartPlay() override;

	int32 GetKillCountAll(const FUniqueNetIdRepl& PlayerUID) const;
	int32 GetKillCountAll(const APlayerState* PlayerState) const;
	int32 GetKillCountAll(FName MonsterRowName) const;

	int32 GetKillCount(const FUniqueNetIdRepl& PlayerUID, FName MonsterRowName) const;
	int32 GetKillCount(const APlayerState* PlayerState, FName MonsterRowName) const;

	float GetStageElapsedTime() const;

	void OnDead(const APawn* InInstigator, const IKillableInterface* Killed);
	void OnMonsterDead(const APlayerState* PlayerState, const IKillableInterface* Killable);

	TObjectPtr<UZombieManager> GetZombieManager();

protected:
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Clear();

	virtual void Initialize();

private:
	void StartStage();
	void EndStage();

	void InitializeStageObjective();

#pragma endregion

#pragma region StageObjective
private:
	UPROPERTY()
	TArray<FStageObjectiveEntry> StageObjectiveEntries;

public:
	const TArray<FStageObjectiveEntry>& GetStageObjectiveEntries() const;
	const FStageObjectiveEntry* GetStageObjectiveEntry(int32 Index) const;

private:
	void HandleKillInfoChanged(const FKillInfo* KillInfo);
#pragma endregion
};
