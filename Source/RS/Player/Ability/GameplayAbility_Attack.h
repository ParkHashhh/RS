// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlayAbility_PlayerBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GameplayAbility_Attack.generated.h"

/**
 *
 */
UCLASS()
class RS_API UGameplayAbility_Attack : public UGameplayAbility_PlayerBase
{
	GENERATED_BODY()
public:
	UGameplayAbility_Attack();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> AttackGameplayEffectClass;

	FActiveGameplayEffectHandle AttackEffectHandle;

	FTimerHandle EmptyAttackTimerHandle;

	bool bStop = false;

protected:
	// 어빌리티가 BeginPlay 등에서 최초 실행될 때 발동하는 함수
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
			const FGameplayAbilityActorInfo* ActorInfo,
			const FGameplayAbilityActivationInfo ActivationInfo,
			const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	void LoopAttackSequence();

	UFUNCTION()
	void OnAttackSequenceStep();


	UFUNCTION()
	void OnInputReleased(float TimeHeld);
};
