// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSTypes.h"
#include "Animation/AnimInstance.h"
#include "ZombieAnimInstance.generated.h"

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
	TMap<FName, TObjectPtr<UAnimSequence>> AnimMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UBlendSpace>> BlendMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UAnimMontage>> MontageMap;

public:
	void SetAnim(EZombieAnimType Type)
	{
		AnimType = Type;
	}

public:
	int32 GetMontageNum()
	{
		return MontageMap.Num();
	}

public:
	virtual void NativeBeginPlay();
	virtual void NativeUpdateAnimation(float DeltaSeconds);

public:
	void PlayAttackMontage(int32 Number);

public:
	UFUNCTION()
	void AnimNotify_Attack();

	UFUNCTION()
	void AnimNotify_AttackEnd();

	UFUNCTION()
	void AnimNotify_Death();
};
