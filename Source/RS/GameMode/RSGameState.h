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
	FName TableRowName;

	UPROPERTY()
	float CurrentProgress;
};

/**
 *
 */
UCLASS()
class RS_API ARSGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ARSGameState();

	// private:
	UPROPERTY(ReplicatedUsing=OnRep_ObjectiveDisplayEntries, VisibleAnywhere, BlueprintReadOnly,
		meta=(AllowPrivateAccess=true))
	TArray<FStageObjectiveDisplayEntry> ObjectiveDisplayEntries;

	UPROPERTY(ReplicatedUsing=OnRep_Objective1, EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	FStageObjective Objective1;

	int32 PrevObjective1Progress = 0;

	UPROPERTY(ReplicatedUsing=OnRep_Objective2, EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	FStageObjective Objective2;

	int32 PrevObjective2Progress = 0;

	UPROPERTY(ReplicatedUsing=OnRep_Objective3, EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess))
	FStageObjective Objective3;

	int32 PrevObjective3Progress = 0;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 StageLevel = 1;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bBossStage = false;

	UPROPERTY(ReplicatedUsing=OnRep_Initialized)
	bool bInitialized = false;

	FOnGameStateReady OnGameStateReadyDelegate;

public:
	TArray<FOnObjectiveStateChanged> OnObjectiveStateChangedDelegates
	{
		FOnObjectiveStateChanged(), FOnObjectiveStateChanged(), FOnObjectiveStateChanged(),
	};

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

	const FStageObjective& GetObjective1() const
	{
		return Objective1;
	}

	void SetObjective1(const FStageObjective& InObjective);

	const FStageObjective& GetObjective2() const
	{
		return Objective2;
	}

	void SetObjective2(const FStageObjective& InObjective);

	const FStageObjective& GetObjective3() const
	{
		return Objective3;
	}

	void SetObjective3(const FStageObjective& InObjective);

	int32 GetStageLevel() const
	{
		return StageLevel;
	}

	bool IsBossStage() const
	{
		return bBossStage;
	}

	// TODO: 데이터로 스테이지 초기화 변경 필요
	void Initialize(const TArray<FStageObjectiveEntry>& Objectives);

protected:
	UFUNCTION()
	void OnRep_Objective1();
	void InnerOnRepObjective1();

	UFUNCTION()
	void OnRep_Objective2();
	void InnerOnRepObjective2();

	UFUNCTION()
	void OnRep_Objective3();
	void InnerOnRepObjective3();

	UFUNCTION()
	void OnRep_Initialized();
	void InnerOnRepInitialized();

private:
	UFUNCTION()
	void OnRep_ObjectiveDisplayEntries();

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

private:
	void OnObjectiveAdded(const FStageObjectiveEntry& Entry);
	void OnObjectiveChanged(const FStageObjectiveEntry& Entry);
	void OnObjectiveRemoved(const FStageObjectiveEntry& Entry);
};
