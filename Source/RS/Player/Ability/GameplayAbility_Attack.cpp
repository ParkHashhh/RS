// Fill out your copyright notice in the Description page of Project Settings.

#include "GameplayAbility_Attack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/LogCategory.h"


UGameplayAbility_Attack::UGameplayAbility_Attack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGameplayAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	bStop = false;
	UAbilityTask_WaitInputRelease* WaitRelease = UAbilityTask_WaitInputRelease::WaitInputRelease(this);

	if (WaitRelease)
	{
		WaitRelease->OnRelease.AddDynamic(this, &UGameplayAbility_Attack::OnInputReleased);
		WaitRelease->ReadyForActivation();
	}


	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (PlayerChar)
	{
		if (AttackGameplayEffectClass)
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddInstigator(PlayerChar, PlayerChar);

			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
				AttackGameplayEffectClass, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				AttackEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			}
		}
	}
	LoopAttackSequence();
}



void UGameplayAbility_Attack::LoopAttackSequence()

{

	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (PlayerChar)
	{
		TObjectPtr<UAnimMontage> AttackMontage = PlayerChar->GetBaseAttackMontage();

		if (!AttackMontage)
		{
			RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("Have Not AttackMontage"));
			return;
		}
		float DefaultAttackSpeed = 0.1f;

		if (PlayerChar->GetAttributeSet())
		{
			DefaultAttackSpeed = PlayerChar->GetAttributeSet()->GetAttackSpeed();
		}

		DefaultAttackSpeed = FMath::Max(DefaultAttackSpeed, 0.01f);

		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, 0.7f);


		Task->OnCompleted.AddDynamic(this, &UGameplayAbility_Attack::OnAttackSequenceStep);
		Task->OnBlendOut.AddDynamic(this, &UGameplayAbility_Attack::OnAttackSequenceStep);
		Task->OnCancelled.AddDynamic(this, &UGameplayAbility_Attack::OnAttackSequenceStep);
		Task->OnInterrupted.AddDynamic(this, &UGameplayAbility_Attack::OnAttackSequenceStep);

		Task->ReadyForActivation();
	}
}



void UGameplayAbility_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("EndAbility Attack"));

	if (ActorInfo->AnimInstance.IsValid())
	{
		ActorInfo->AnimInstance->Montage_Stop(0.2f);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Attack::OnAttackSequenceStep()
{
	if (bStop)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		LoopAttackSequence();
	}
}

void UGameplayAbility_Attack::OnInputReleased(float TimeHeld)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Combat.Attacking"));
		ASC->RemoveLooseGameplayTag(AttackTag); // 혹은 ActiveGE 제거
	}
	bStop = true;
}

