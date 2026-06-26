// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Reload.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/LogCategory.h"


UGameplayAbility_Reload::UGameplayAbility_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

}

void UGameplayAbility_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
		WaitRelease->OnRelease.AddDynamic(this, &UGameplayAbility_Reload::OnFinishReload);
		WaitRelease->ReadyForActivation();
	}
	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (PlayerChar)
	{
		if (ReloadGameplayEffect)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddInstigator(PlayerChar, PlayerChar);

			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				ReloadGameplayEffect, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				ReloadffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			}
		}
	}

		TObjectPtr<UAnimMontage> ReloadMontage = PlayerChar->GetReloadMontage();

		if (!ReloadMontage)
		{
			RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("Have Not ReloadMontage"));
			return;
		}
		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, ReloadMontage,1.f);

		Task->ReadyForActivation();
}



void UGameplayAbility_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("EndAbility Dash"));

	if (ReloadffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		ASC->RemoveActiveGameplayEffect(ReloadffectHandle);
		ReloadffectHandle.Invalidate();
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Reload::OnFinishReload(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

