// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/GAS/Ability/GameplayAbility_Aim.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Core/LogCategory.h"
#include "RS/Player/Character/PlayerCharacterBase.h"

UGameplayAbility_Aim::UGameplayAbility_Aim()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGameplayAbility_Aim::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayTagContainer* SourceTags,
                                              const FGameplayTagContainer* TargetTags,
                                              FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// TODO: 조준 가능 여부 확인

	return true;
}

void UGameplayAbility_Aim::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_WaitInputRelease* WaitRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (!WaitRelease)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	WaitRelease->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
	WaitRelease->ReadyForActivation();

	if (ActorInfo->OwnerActor.IsValid())
	{
		if (auto* PlayerChar = Cast<APlayerCharacterBase>(ActorInfo->OwnerActor.Get()))
		{
			if (AimGameplayEffectClass)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddInstigator(PlayerChar, PlayerChar);

				const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
					AimGameplayEffectClass, 1.0f, EffectContext);
				if (SpecHandle.IsValid())
				{
					AimingEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
				}
			}
		}
	}
}

void UGameplayAbility_Aim::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      bool bReplicateEndAbility, bool bWasCancelled)
{
	RS_LOG_DEBUG(LogRSGAS, Log, TEXT("%s: EndAbility"), *LOG_LINE_INFO);

	if (ActorInfo->OwnerActor.IsValid())
	{
		if (auto* PlayerChar = Cast<APlayerCharacterBase>(ActorInfo->OwnerActor.Get()))
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			if (ASC && AimingEffectHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(AimingEffectHandle);
				AimingEffectHandle.Invalidate();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Aim::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
