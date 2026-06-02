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

	mCoolDown = 0.2f;
	mCoolDownTag = FGameplayTag::RequestGameplayTag(TEXT("Player.Battle.Attack.CoolDown"));
}


void UGameplayAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 부모 검사에서 쿨타임 때문에 걸러졌다면 즉시 종료
	if (!mAbilityActive)
	{
		return;
	}

	// 🟢 [쿨타임 통과 완료] 이제 여기서 진짜 공격 연출을 합니다!
	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(GetAvatarActorFromActorInfo());
	if (PlayerChar)
	{
		// 예: 캐릭터의 공격 애니메이션 몽타주 재생 함수 호출
		// PlayerChar->PlayAttackMontage();
	}

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Green, TEXT("🟢 공격 스킬 진짜 발동 성공!!!"));

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);

}
