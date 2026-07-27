// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_ZombieBoss_EarlyCooldown.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RS/Zombie/GAS/ZombieStandardAttributeSet.h"
#include "RS/Zombie/GAS/ZombieBossAttributeSet.h"
#include "RS/LogCategory.h"
#include "RS/Zombie/ZombieSpecial.h"
#include "GameplayTagContainer.h"

UGA_ZombieBoss_EarlyCooldown::UGA_ZombieBoss_EarlyCooldown()
{
}

void UGA_ZombieBoss_EarlyCooldown::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CommitAbility(Handle, ActorInfo, ActivationInfo);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_ZombieBoss_EarlyCooldown::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//등록해 둔 쿨타임 이펙트를 가져온다.
	UGameplayEffect* CDEffect = GetCooldownGameplayEffect();

	//좀비의 쿨타임 수치를 가져온다.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UZombieBossAttributeSet* Attri = ASC->GetSet<UZombieBossAttributeSet>();

	//스킬 넘버를 통해 쿨타임을 얻어온다.
	float CoolTime = 0.f;
	switch (SkillNumber)
	{
	case 1:
		CoolTime = Attri->GetBoss_Skill1Cooldown();
		break;
	case 2:
		CoolTime = Attri->GetBoss_Skill2Cooldown();
		break;
	case 3:
		CoolTime = Attri->GetBoss_Skill3Cooldown();
		break;
	case 4:
		CoolTime = Attri->GetBoss_Skill4Cooldown();
		break;
	case 5:
		CoolTime = Attri->GetBoss_Skill5Cooldown();
		break;
	case 6:
		CoolTime = Attri->GetBoss_Skill6Cooldown();
		break;
	}

	//쿨타임 이펙트 스펙핸들을 만든다.
	FGameplayEffectSpecHandle CDSpec = MakeOutgoingGameplayEffectSpec(CDEffect->GetClass(), GetAbilityLevel());

	//스펙 핸들에 setbycaller로 넣어줄 값을 지정한다.
	CDSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.BZSkillCooldownDuration")), CoolTime);

	//넣어준 스킬 쿨타임 태그가 유효하다면 적용한다.
	if (SkillCooldownTag.IsValid())
	{
		CDSpec.Data->DynamicGrantedTags.AddTag(SkillCooldownTag);
	}
	
	//나 자신에게 쿨타임 이펙트를 적용한다.
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CDSpec);
}
