// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RS/RSTypes.h"
#include "RS/RSUtils.h"
#include "RSGameState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnGameStateReady)

DECLARE_MULTICAST_DELEGATE(FOnObjective1StateChanged)
DECLARE_MULTICAST_DELEGATE(FOnObjective2StateChanged)
DECLARE_MULTICAST_DELEGATE(FOnObjective3StateChanged)

UENUM(BlueprintType)
enum class EStageState : uint8
{
	Preparing,
	Playing,
	Cleared,
	Failed,
	Traveling,
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

protected:
	UPROPERTY(ReplicatedUsing=OnRep_StageState, BlueprintReadOnly)
	EStageState StageState;

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

	FOnObjective1StateChanged OnObjective1StateChangedDelegate;
	FOnObjective2StateChanged OnObjective2StateChangedDelegate;
	FOnObjective3StateChanged OnObjective3StateChangedDelegate;

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

	EStageState GetStageState() const
	{
		return StageState;
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
	void Initialize();

protected:
	UFUNCTION()
	void OnRep_StageState();

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
};
