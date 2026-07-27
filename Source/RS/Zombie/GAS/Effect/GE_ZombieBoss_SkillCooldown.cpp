// Fill out your copyright notice in the Description page of Project Settings.


#include "GE_ZombieBoss_SkillCooldown.h"




/*

*/

UGE_ZombieBoss_SkillCooldown::UGE_ZombieBoss_SkillCooldown()
{
	// 얼마나 지속될 것인지를 결정한다.
	// Instant : 즉시 동작.
	// HasDuration : 일정시간 동작.
	// Infinite : 무한하게 동작.
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat	Caller;

	Caller.DataTag = FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.BZSkillCooldownDuration"));

	DurationMagnitude = FGameplayEffectModifierMagnitude(Caller);
}
