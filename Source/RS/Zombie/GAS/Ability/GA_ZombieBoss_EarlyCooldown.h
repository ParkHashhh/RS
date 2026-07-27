// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Zombie/GAS/Ability/GameplayAbility_ZombieBase.h"
#include "GA_ZombieBoss_EarlyCooldown.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGA_ZombieBoss_EarlyCooldown : public UGameplayAbility_ZombieBase
{
	GENERATED_BODY()

public:
	UGA_ZombieBoss_EarlyCooldown();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	FGameplayTag SkillCooldownTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	int32 SkillNumber = 0;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData);

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const;
	
};
