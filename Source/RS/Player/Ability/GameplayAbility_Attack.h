// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GAS/GamePlayAbility_PlayerBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayAbility_Attack.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayAbility_Attack : public UGameplayAbility_PlayerBase
{
	GENERATED_BODY()public:
		UGameplayAbility_Attack();

		// 어빌리티가 BeginPlay 등에서 최초 실행될 때 발동하는 함수
		virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo,
			const FGameplayEventData* TriggerEventData) override;

protected:
	FTimerHandle AttackTimerHandle;

	void LoopAttackSequence();
};
