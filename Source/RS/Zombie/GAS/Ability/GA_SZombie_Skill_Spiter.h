// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Zombie/GAS/Ability/GameplayAbility_ZombieBase.h"
#include "GA_SZombie_Skill_Spiter.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGA_SZombie_Skill_Spiter : public UGameplayAbility_ZombieBase
{
	GENERATED_BODY()

public:
	UGA_SZombie_Skill_Spiter();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData);

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const;
	
};
