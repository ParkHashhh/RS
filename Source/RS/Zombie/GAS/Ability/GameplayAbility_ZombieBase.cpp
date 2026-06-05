// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_ZombieBase.h"

UGameplayAbility_ZombieBase::UGameplayAbility_ZombieBase()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGameplayAbility_ZombieBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}


