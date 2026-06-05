// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Attack.h"
#include "AbilitySystemComponent.h"
#include "RS/Player/PlayerCharacterBase.h"

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

	// 부모 검사에서 쿨타임 때문에 걸러졌다면 즉시 종료
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}


	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (PlayerChar)
	{
		if (PlayerChar->IsLocallyControlled() || PlayerChar->HasAuthority())
		{
			PlayerChar->ShootAttackRay();
		}

		// 캐릭터의 공격 애니메이션 몽타주 재생 함수 호출
		// PlayerChar->PlayAttackMontage();
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);

}
