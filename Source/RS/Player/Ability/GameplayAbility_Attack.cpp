// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Attack.h"
#include "AbilitySystemComponent.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"
#include "RS/LogCategory.h"

UGameplayAbility_Attack::UGameplayAbility_Attack()
{
	FAbilityTriggerData	TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Battle.Attack"));
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
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

	LoopAttackSequence();


}

void UGameplayAbility_Attack::LoopAttackSequence()
{

	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());

	if (PlayerChar && PlayerChar->IsAttacking())
	{
		if (PlayerChar->IsLocallyControlled())
		{
			PlayerChar->ShootAttackRay();

			// 캐릭터의 공격 애니메이션 몽타주 재생 함수 호출
			PlayerChar->PlayBaseAttackMontage();
		}

		float DefaultAttackSpeed = 1.0f;

		if (PlayerChar->GetAttributeSet())
		{
			DefaultAttackSpeed = PlayerChar->GetAttributeSet()->GetAttackSpeed();
		}

		float BaseDelay = 0.2f;
		float FinalDelay = BaseDelay / FMath::Max(DefaultAttackSpeed, 0.01f);

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(AttackTimerHandle, this,
				&UGameplayAbility_Attack::LoopAttackSequence, FinalDelay, false);
		}

	}

	else
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AttackTimerHandle);
		}
		// 마우스를 뗐다면 어빌리티 종료
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
