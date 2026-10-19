// Fill out your copyright notice in the Description page of Project Settings.

#include "RS/Player/GAS/Ability/GameplayAbility_Attack.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/GAS/Attribute/PlayerAttributeSet.h"
#include "RS/Core/LogCategory.h"


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
		if (!PlayerChar->UseAmmo())
		{
			float AttackInterval = 0.25f;
			GetWorld()->GetTimerManager().SetTimer(EmptyAttackTimerHandle, this, &UGameplayAbility_Attack::OnAttackSequenceStep, AttackInterval, false);
			return;
		}
		TObjectPtr<UAnimMontage> AttackMontage = PlayerChar->GetBaseAttackMontage();

		FVector MuzzleLocation = PlayerChar->GetWeaponComponent()->GetSocketLocation(FName("Muzzle"));
		FRotator MuzzleRotation = PlayerChar->GetWeaponComponent()->GetSocketRotation(FName("Muzzle"));	

		PlayerChar->ExecuteMuzzleEffect(MuzzleLocation, MuzzleRotation);

		if (!AttackMontage)
		{
			RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("Have Not AttackMontage"));
			return;
		}
	
		// 공격속도 = 1 + (AttackSpeed% / 100). 아이템이 0~100%로 누적되며, 100%를 채우면 2배 속도가 된다.
		float AttackSpeedStat = 0.f;
		if (PlayerChar->GetAttributeSet())
		{
			AttackSpeedStat = PlayerChar->GetAttributeSet()->GetAttackSpeed();
		}

		const float MontagePlayRate = FMath::Clamp(1.0f + (AttackSpeedStat / 100.0f), 1.0f, 2.0f);

		UAbilityTask_PlayMontageAndWait* Task =
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, MontagePlayRate);

		Task->OnCompleted.AddDynamic(this, &UGameplayAbility_Attack::OnAttackSequenceStep);

		Task->ReadyForActivation();
	}
}



void UGameplayAbility_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Combat.Attacking"));
		ASC->RemoveLooseGameplayTag(AttackTag);
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Attack::OnAttackSequenceStep()
{
	if (bStop || !IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
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
		ASC->RemoveLooseGameplayTag(AttackTag);
	}
	bStop = true;
}

