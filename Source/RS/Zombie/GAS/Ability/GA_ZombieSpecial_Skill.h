// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Zombie/GAS/Ability/GameplayAbility_ZombieBase.h"
#include "GA_ZombieSpecial_Skill.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGA_ZombieSpecial_Skill : public UGameplayAbility_ZombieBase
{
	GENERATED_BODY()

public:
	UGA_ZombieSpecial_Skill();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData);

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled);

	
};
