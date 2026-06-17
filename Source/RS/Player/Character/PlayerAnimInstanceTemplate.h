// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/Character/PlayerAnimInstance.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "PlayerAnimInstanceTemplate.generated.h"
/*

anim_m4a1_stand_idle -> StandIdle

bs_startloco -> StartLoco
bs_looploco -> LoopLoco
bs_endloco -> EndLoco
bs_aimlooploco -> AimLoopLoco

anim_m4a1_jump -> JumpStart
anim_m4a1_jump_fall_loop -> JumpApex
anim_m4a1_jump_land -> JumpLand

AimOffset
PlayerFullBody 슬롯으로 만든 BaseAttack ->AttackMontage

*/
UCLASS()
class RS_API UPlayerAnimInstanceTemplate : public UPlayerAnimInstance
{
	GENERATED_BODY()


protected:
	// 일반 애니메이션 (Idle, Jump 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TMap<FName, TObjectPtr<UAnimSequence>> AnimSequenceMap;

	// 블렌드 스페이스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TMap<FName, TObjectPtr<UBlendSpace>> BlendSpaceMap;

	// 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TMap<FName, TObjectPtr<UAnimMontage>> MontageMap;

	// 에임 오프셋 (별도 관리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAimOffsetBlendSpace> AimOffset;
public:
	virtual void NativeBeginPlay();
	virtual void NativeUpdateAnimation(float DeltaSeconds);
};
