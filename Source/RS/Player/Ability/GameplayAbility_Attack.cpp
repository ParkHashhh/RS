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

	if (PlayerChar)
	{
		if (PlayerChar->IsLocallyControlled() && PlayerChar->IsAttacking())
		{
			// 캐릭터의 공격 애니메이션 몽타주 재생 함수 호출
			PlayerChar->PlayBaseAttackMontage();

			float DefaultAttackSpeed = 1.0f;

			if (PlayerChar->GetAttributeSet())
			{
				DefaultAttackSpeed = PlayerChar->GetAttributeSet()->GetAttackSpeed();
			}

			float BaseDelay = 0.2f;
			float FinalDelay = BaseDelay / FMath::Max(1, 0.01f);

			GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, this, &UGameplayAbility_Attack::LoopAttackSequence, FinalDelay, false);
		}
		else
		{
			RS_PRINT_SCREEN(0.2f, FColor::Blue, TEXT("End Ability"));

			GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
		}
	}
}
