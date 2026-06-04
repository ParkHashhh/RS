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

	// 변수 초기화 기본값
	mCoolDownClass = nullptr;
	mCoolDown = 0.f;
	mAbilityActive = false;
}
const FGameplayTagContainer* UGameplayAbility_Base::GetCooldownTags() const
{
	mCoolDownTags.Reset();
	const FGameplayTagContainer* ParentTags = Super::GetCooldownTags();
	if (ParentTags)
	{
		mCoolDownTags.AppendTags(*ParentTags);
	}
	if (mCoolDownTag.IsValid())
	{
		mCoolDownTags.AddTag(mCoolDownTag);
	}
	return &mCoolDownTags;
}

void UGameplayAbility_Base::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{

	if (!mCoolDownClass || !ActorInfo)
		return;

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
		return;

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(mCoolDownClass, GetAbilityLevel(Handle, ActorInfo));
	if (!SpecHandle.IsValid())
		return;

	SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("Data.CoolDown")), mCoolDown);

	// 이 쿨타임이 도는 동안 내 몸에 붙어있을 고유 태그 부여
	if (mCoolDownTag.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AddTag(mCoolDownTag);
	}

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
}

void UGameplayAbility_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		mAbilityActive = false;
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 🎯 CommitAbility를 하는 순간 자동으로 쿨타임 상태를 체크하고, 
	// 쿨타임 중이 아니라면 ApplyCooldown을 호출해 즉시 쿨타임을 돌려버립니다.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		mAbilityActive = false;
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 🟢 쿨타임 통과 완료! 실제 액션 로직 시작 가능 상태
	mAbilityActive = true;


}
