// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_ZombieBase.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayAbility_ZombieBase : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_ZombieBase();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData);

public:
	const TArray<FAbilityTriggerData>& GetAbilityTriggers()	const
	{
		return AbilityTriggers;
	}
	
};
