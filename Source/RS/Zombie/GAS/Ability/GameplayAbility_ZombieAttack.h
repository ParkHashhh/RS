// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayAbility_ZombieBase.h"
#include "GameplayAbility_ZombieAttack.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayAbility_ZombieAttack : public UGameplayAbility_ZombieBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_ZombieAttack();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData);
	
};
