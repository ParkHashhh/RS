// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_ZombieAttack.h"
#include "../../../GAS/BaseAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"

UGameplayAbility_ZombieAttack::UGameplayAbility_ZombieAttack()
{
}

void UGameplayAbility_ZombieAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!TriggerEventData || !TriggerEventData->Target)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayAbilityTargetData_SingleTargetHit* HitData =
		(FGameplayAbilityTargetData_SingleTargetHit*)(TriggerEventData->TargetData.Data[0].Get());
	if (!HitData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	//공격자와 피격자의 attributeSet을 가져온다.
	AActor* SourceActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	const UBaseAttributeSet* SourceAttri = SourceASC->GetSet<UBaseAttributeSet>();

	AActor* TargetActor = HitData->HitResult.GetActor();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!SourceASC || !TargetASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	//보통 피격자의 AttributeSet이 변화하는 경우가 대부분이므로 크래시를 방지하기 위해 방어 구문을 추가한다.
	const UBaseAttributeSet* TargetAttri = TargetASC->GetSet<UBaseAttributeSet>();

	if (!TargetAttri)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayEffectContextHandle Context = MakeEffectContext(Handle, ActorInfo);
	Context.AddHitResult(HitData->HitResult);

	float Attack = SourceAttri->GetAttack();

	FString DebugMessage = FString::Printf(TEXT("Zombie Attack! Damage = %.2f"), Attack);
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, DebugMessage);

	//Damage 이펙트를 실행한다.


	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
