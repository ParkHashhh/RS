// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RS/Core/RSTypes.h"
#include "RS/Core/RSUtils.h"
#include "RSGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnGameStateReady)

DECLARE_MULTICAST_DELEGATE(FOnObjectiveStateChanged);
DECLARE_MULTICAST_DELEGATE(FOnObjectiveChanged);

//로그인/로그아웃 신호를 받으면 호출될 함수를 연결하는 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnPartyListChange);

USTRUCT(BlueprintType)
struct RS_API FStageObjectiveDisplayEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FName TableRowName{};

	UPROPERTY()
	float CurrentProgress{};
};

class AZombieBoss;
/**
 *
 */
UCLASS()
class RS_API ARSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ARSGameState();

private:

	UPROPERTY(ReplicatedUsing=OnRep_Initialized)
	bool bInitialized = false;

	FOnGameStateReady OnGameStateReadyDelegate;

	//현재 존재하는 보스를 보관할 멤버 변수
	UPROPERTY(ReplicatedUsing = OnRep_CheckZombieBoss)
	AZombieBoss* CurrentBoss = nullptr;

	UPROPERTY(Replicated)
	int32 DifficultyStage = 1;

public:
	int32 GetDifficultyStage() const
	{
		return DifficultyStage;
	}

	// 인원수 배율: 1인->1.0, 2인->1.5, 3인->1.8, 4인->2.0. 5인 이상은 마지막 값을 유지한다.
	float GetPlayerCountMultiplier() const
	{
		static const TArray<float> Multipliers = {1.0f, 1.0f, 1.5f, 1.8f, 2.0f};
		const int32 Count = FMath::Max(PlayerArray.Num(), 1);
		return Multipliers[FMath::Min(Count, Multipliers.Num() - 1)];
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	template <typename T>
	void ExecuteOrBindOnGameStateReady(T* Object, void (T::*Callback)())
	{
		RSUtils::ExecuteOrBind(bInitialized, OnGameStateReadyDelegate, Object, Callback);
	}

	template <typename T>
	void RemoveAllOnGameStateReady(T* Object)
	{
		static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "T must be derived from UObject");
		OnGameStateReadyDelegate.RemoveAll(Object);
	}

	void Initialize(const TArray<FStageObjectiveEntry>& Objectives);
	void SetDifficultyStage(int32 InStage);
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnRep_Initialized();
	void InnerOnRepInitialized();
	UFUNCTION()
	void OnRep_CheckZombieBoss();

public:
	void ChangeBossHPBarActivation();

private:
	UPROPERTY(Replicated)
	FStageObjectiveList ObjectiveList;

public:
	FOnObjectiveChanged OnObjectiveChangedDelegate;

	//인원 변화를 감지하면 사용할 델리게이트
	FOnPartyListChange OnPartyListChangeDelegate;

public:
	FStageObjectiveEntry* AddObjective(FName TableRowName, float CurrentProgress, int32 DisplayOrder);

	void SetProgress(FName TableRowName, float NewProgress);

	FStageObjectiveEntry* FindObjective(FName TableRowName);
	const FStageObjectiveEntry* FindObjective(FName TableRowName) const;

	float GetObjectiveProgress(FName TableRowName) const;
	float GetObjectiveProgressRatio(FName TableRowName) const;

	void HandleEntryAdded(const FStageObjectiveEntry& Entry);
	void HandleEntryChanged(const FStageObjectiveEntry& Entry);
	void HandleEntryRemoved(const FStageObjectiveEntry& Entry);

	const TArray<FStageObjectiveEntry>& GetObjectiveEntries() const
	{
		return ObjectiveList.Entries;
	}

	const FStageObjectiveEntry* GetObjectiveEntry(int32 Index) const
	{
		return ObjectiveList.Entries.IsValidIndex(Index) ? &ObjectiveList.Entries[Index] : nullptr;
	}

	const FStageObjectiveEntry* GetObjectiveEntry(FName TableRowName) const
	{
		if (TableRowName.IsNone())
		{
			return nullptr;
		}

		const int32* Found = ObjectiveList.TableRowNameIndexMap.Find(TableRowName);
		return Found ? &ObjectiveList.Entries[*Found] : nullptr;
	}

	void RegistBoss(AZombieBoss* Boss);
	void UnRegistBoss(AZombieBoss* Boss);
	AZombieBoss* GetCurrentBoss() const
	{
		return CurrentBoss;
	}

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PartyListRefresh();

private:
	void OnObjectiveAdded(const FStageObjectiveEntry& Entry);
	void OnObjectiveChanged(const FStageObjectiveEntry& Entry);
	void OnObjectiveRemoved(const FStageObjectiveEntry& Entry);
};
