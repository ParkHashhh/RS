// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Dash.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/LogCategory.h"


UGameplayAbility_Dash::UGameplayAbility_Dash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UGameplayAbility_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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


	UAbilityTask_WaitInputRelease* WaitRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this);

	if (WaitRelease)
	{
		WaitRelease->OnRelease.AddDynamic(this, &UGameplayAbility_Dash::OnInputReleased);
		WaitRelease->ReadyForActivation();
	}




	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());



	if (PlayerChar)
	{
		if (DashGameplayEffectClass)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddInstigator(PlayerChar, PlayerChar);

			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				DashGameplayEffectClass, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				DashEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			}
		}
	}
}



void UGameplayAbility_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (DashEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		ASC->RemoveActiveGameplayEffect(DashEffectHandle);
		DashEffectHandle.Invalidate();
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}



void UGameplayAbility_Dash::OnInputReleased(float TimeHeld)
{

	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (PlayerChar)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC && DashEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(DashEffectHandle);
			DashEffectHandle.Invalidate();
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
