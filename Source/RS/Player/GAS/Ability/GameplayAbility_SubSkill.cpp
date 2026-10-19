// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/GAS/Ability/GameplayAbility_SubSkill.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/GAS/Attribute/PlayerAttributeSet.h"
#include "RS/Core/LogCategory.h"


UGameplayAbility_SubSkill::UGameplayAbility_SubSkill()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UGameplayAbility_SubSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)

{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))

	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (PlayerChar)
	{
		if (SubSkillGameplayEffect)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddInstigator(PlayerChar, PlayerChar);

			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				SubSkillGameplayEffect, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				SubSkillEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			}
		}
	}

	// 여기만 Grenade로 바꿔주기
	TObjectPtr<UAnimMontage> SubSkillMontage = PlayerChar->GetSubSkillMontage();

	if (!SubSkillMontage)
	{
		return;
	}
	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SubSkillMontage, 1.0f);
	Task->OnCompleted.AddDynamic(this, &UGameplayAbility_SubSkill::OnFinishSubSkill);
	Task->OnInterrupted.AddDynamic(this, &UGameplayAbility_SubSkill::OnFinishSubSkill);
	Task->OnCancelled.AddDynamic(this, &UGameplayAbility_SubSkill::OnFinishSubSkill);
	Task->OnBlendOut.AddDynamic(this, &UGameplayAbility_SubSkill::OnFinishSubSkill);

	Task->ReadyForActivation();

}



void UGameplayAbility_SubSkill::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{

	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (SubSkillEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		ASC->RemoveActiveGameplayEffect(SubSkillEffectHandle);
		SubSkillEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (IsValid(PlayerChar))
	{
		PlayerChar->VisibleWeapon();
		if (PlayerChar->IsAttackKeyPressed())
		{
			PlayerChar->CallAttackAbility();
		}
	}
}

void UGameplayAbility_SubSkill::OnFinishSubSkill()
{
	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}


