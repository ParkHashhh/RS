// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Grenade.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/LogCategory.h"


UGameplayAbility_Grenade::UGameplayAbility_Grenade()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UGameplayAbility_Grenade::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	UAbilityTask_WaitInputRelease* WaitRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this);

	if (WaitRelease)
	{
		WaitRelease->OnRelease.AddDynamic(this, &UGameplayAbility_Grenade::OnFinishGrenade);
		WaitRelease->ReadyForActivation();
	}
	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (PlayerChar)
	{
		if (GrenadeGameplayEffect)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddInstigator(PlayerChar, PlayerChar);

			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				GrenadeGameplayEffect, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				GrenadeEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
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

	Task->ReadyForActivation();
}



void UGameplayAbility_Grenade::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("EndAbility SubSkilling"));

	if (GrenadeEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		ASC->RemoveActiveGameplayEffect(GrenadeEffectHandle);
		GrenadeEffectHandle.Invalidate();
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Grenade::OnFinishGrenade(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


