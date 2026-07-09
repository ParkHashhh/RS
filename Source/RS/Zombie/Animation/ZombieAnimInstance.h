// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSTypes.h"
#include "Animation/AnimInstance.h"
#include "ZombieAnimInstance.generated.h"

USTRUCT(BlueprintType)
struct FAnimPreset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UAnimSequence>> AnimMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UBlendSpace>> BlendMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UAnimMontage>> AtkMontageMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UAnimMontage>> SkillMontageMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UAnimMontage>> CommonMontageMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float WalkNativeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunNativeSpeed;
};

UCLASS()
class RS_API UZombieAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UZombieAnimInstance();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EZombieAnimType AnimType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FAnimPreset> AnimPresetArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float TurnSpeedRate_Walk;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsTurn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int32 AnimIndex_Backup = 0;

public:
	void SetAnim(EZombieAnimType Type)
	{
		AnimType = Type;
	}
	void SetAnimPresetIndexBackup(int32 Index)
	{
		AnimIndex_Backup = Index;
	}

public:
	int32 GetAtkMontageNum()
	{
		return AnimPresetArray[AnimIndex_Backup].AtkMontageMap.Num();
	}
	int32 GetSkillMontageNum()
	{
		return AnimPresetArray[AnimIndex_Backup].SkillMontageMap.Num();
	}
	bool IsAnimPresetArrayEmpty() const
	{
		return AnimPresetArray.IsEmpty();
	}
	int32 GetAnimPresetNum() const
	{
		return AnimPresetArray.Num();
	}

public:
	virtual void NativeBeginPlay();
	virtual void NativeUpdateAnimation(float DeltaSeconds);

public:
	void PlayAttackMontage(int32 Number);
	void PlaySkillMontage(int32 Number);
	void PlayCommonMontage(const FName& Name);
	void SetIsTurn(bool IsTurn);
	bool ChechRoarAnimValid();
	void FinishJump();

public:
	UFUNCTION()
	void AnimNotify_Attack();

	UFUNCTION()
	void AnimNotify_AttackEnd();

	UFUNCTION()
	void AnimNotify_Death();

	UFUNCTION()
	void AnimNotify_RoarStart();

	UFUNCTION()
	void AnimNotify_RoarEnd();

	UFUNCTION()
	void AnimNotify_RFootStep();

	UFUNCTION()
	void AnimNotify_LFootStep();

	UFUNCTION()
	void AnimNotify_SkillStart();

	UFUNCTION()
	void AnimNotify_SkillEnd();

	UFUNCTION()
	void AnimNotify_MoveStart();
};
