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
		// 캐릭터의 공격 애니메이션 몽타주 가져오기
		TObjectPtr<UAnimMontage> AttackMontage = PlayerChar->GetBaseAttackMontage();

		if (!AttackMontage)
		{
			RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("Have Not AttackMontage"));

		}
		float DefaultAttackSpeed = 1.0f;

		if (PlayerChar->GetAttributeSet())
		{
			DefaultAttackSpeed = PlayerChar->GetAttributeSet()->GetAttackSpeed();
		}

		DefaultAttackSpeed = FMath::Max(DefaultAttackSpeed, 0.01f);

		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage);

		Task->OnCompleted.AddDynamic(this, &UGameplayAbility_Attack::OnAttackSequenceStep);

		Task->ReadyForActivation();
	}
}



void UGameplayAbility_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("EndAbility Attack"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Attack::OnAttackSequenceStep()
{
	LoopAttackSequence();

}

void UGameplayAbility_Attack::OnInputReleased(float TimeHeld)
{

	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (PlayerChar)
	{
		UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
		if (ASC && AttackEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(AttackEffectHandle);
			AttackEffectHandle.Invalidate();
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
