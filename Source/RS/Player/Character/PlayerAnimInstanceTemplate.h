// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/Character/PlayerAnimInstance.h"
#include "PlayerAnimInstanceTemplate.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UPlayerAnimInstanceTemplate : public UPlayerAnimInstance
{
	GENERATED_BODY()


protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UAnimSequence>>	mAnimMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UBlendSpace>>	mBlendSpaceMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UAnimMontage>>	mSkillMap;

public:
	virtual void NativeBeginPlay();
	virtual void NativeUpdateAnimation(float DeltaSeconds);
};
