// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/GAS/Ability/GameplayAbility_Reload.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/GAS/Attribute/PlayerAttributeSet.h"
#include "RS/Core/LogCategory.h"


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
				ReloadEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
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

		Task->OnCompleted.AddDynamic(this, &UGameplayAbility_Reload::OnFinishReload);
		Task->OnInterrupted.AddDynamic(this, &UGameplayAbility_Reload::OnFinishReload);
		Task->OnCancelled.AddDynamic(this, &UGameplayAbility_Reload::OnFinishReload);
		Task->OnBlendOut.AddDynamic(this, &UGameplayAbility_Reload::OnFinishReload);

		Task->ReadyForActivation();
}



void UGameplayAbility_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ReloadEffectHandle.IsValid())
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		ASC->RemoveActiveGameplayEffect(ReloadEffectHandle);
		ReloadEffectHandle.Invalidate();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);


	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (IsValid(PlayerChar) && PlayerChar->IsAttackKeyPressed())
	{
		PlayerChar->CallAttackAbility();
	}
}

void UGameplayAbility_Reload::OnFinishReload()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

