// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/GAS/Ability/GameplayAbility_PlayerBase.h"
#include "GameplayAbility_Jump.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayAbility_Jump : public UGameplayAbility_PlayerBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Jump();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> JumpGameplayEffect;

	FActiveGameplayEffectHandle JumpEffectHandle;
protected:
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
	UFUNCTION()
	void OnFinishJump(EMovementMode MovementMode);
};
