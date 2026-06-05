// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_PlayerBase.h"

UGameplayAbility_PlayerBase::UGameplayAbility_PlayerBase()
{
}


void UGameplayAbility_PlayerBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
