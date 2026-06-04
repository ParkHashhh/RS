// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Base.h"
#include "AbilitySystemComponent.h"

UGameplayAbility_Base::UGameplayAbility_Base()
{
	// GAS에서 Ability가 어디에서 실행될지를 결정한다.
	// LocalPredicted : 클라이언트 즉시 실행. 동시에 서버에도 요청. 서버가 최종 판정
	// LocalOnly : 로컬에서만 실행한다. 네트워크 전송 X
	// ServerOnly : 서버에서만 실행한다. 클라 입력 -> 서버 요청 -> 서버만 실행.
	// ServerInitiated : 서버 시작. 서버가 강제로 발동하는 Ability.
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

}


void UGameplayAbility_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

}
