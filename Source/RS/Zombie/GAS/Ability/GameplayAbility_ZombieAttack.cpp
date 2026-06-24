// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_ZombieAttack.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RS/LogCategory.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "RS/Zombie/ZombieBase.h"

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

	//Damage Effect를 발동시키기 위해 Spec Handle을 만든다.
	FGameplayEffectContextHandle Context = MakeEffectContext(Handle, ActorInfo);
	Context.AddHitResult(HitData->HitResult);

	AZombieBase* ZombieActor = Cast<AZombieBase>(SourceActor);
	TSubclassOf<UGameplayEffect> DamageEffect = ZombieActor->GetEffect(TEXT("Damage"));

	if (!IsValid(DamageEffect))
	{
		RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("Damage Effect를 지정하지 않았습니다."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
	DamageSpec.Data->SetContext(Context);

	//데미지를 계산한다
	//최솟값 1 보정이 들어간다.
	float Attack = SourceAttri->GetAttackPower();
	float Defense = TargetAttri->GetDefensePower();
	float Damage = Attack - Defense;
	Damage = FMath::Max(1.f, Damage);

	//변화시킬 값을 가리키는 태그, 변화시킬 값을 이펙트 SpecHandle에 세팅한다.
	DamageSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.HP")), -Damage);

	//타겟 전송을 위해 타겟 데이터, 타겟 배열을 생성
	FGameplayAbilityTargetDataHandle TargetData;
	FGameplayAbilityTargetData_ActorArray* TargetArray = new FGameplayAbilityTargetData_ActorArray;

	//타겟 배열에 타겟을 넣은 후 타겟 데이터에 타겟 배열 추가
	TargetArray->TargetActorArray.Add(TargetActor);
	TargetData.Add(TargetArray);

	//생성한 이펙트 SpecHandle을 이용해 이펙트를 발동시킨다.
	ApplyGameplayEffectSpecToTarget(Handle, ActorInfo, ActivationInfo, DamageSpec, TargetData);

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
