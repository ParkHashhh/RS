// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RS/RSTypes.h"
#include "RS/RSUtils.h"
#include "RSGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnGameStateReady)

DECLARE_MULTICAST_DELEGATE(FOnObjectiveStateChanged);
DECLARE_MULTICAST_DELEGATE(FOnObjectiveChanged);

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

public:
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

protected:
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

private:
	void OnObjectiveAdded(const FStageObjectiveEntry& Entry);
	void OnObjectiveChanged(const FStageObjectiveEntry& Entry);
	void OnObjectiveRemoved(const FStageObjectiveEntry& Entry);
};
