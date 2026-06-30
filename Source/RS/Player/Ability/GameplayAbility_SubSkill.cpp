// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_SubSkill.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/LogCategory.h"


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


	RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("Activate UGameplayAbility_Grenade"));

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
	TObjectPtr<UAnimMontage> GrenadeMontage = PlayerChar->GetSubSkillMontage();

	if (!GrenadeMontage)
	{
		RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("Have Not GetSubSkillMontage"));
		return;
	}
	UAbilityTask_PlayMontageAndWait* Task =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GrenadeMontage, 1.5f);
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
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("EndAbility SubSkilling %d "), PlayerChar->IsAttackKeyPressed());

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


