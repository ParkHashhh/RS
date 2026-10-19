// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_ZombieBoss_Skill4.h"
#include "RS/Zombie/GAS/ZombieBossAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RS/Zombie/Boss/ZombieBoss.h"
#include "RS/Core/LogCategory.h"

UGA_ZombieBoss_Skill4::UGA_ZombieBoss_Skill4()
{

}

void UGA_ZombieBoss_Skill4::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	//등록한 스킬 쿨타임 이펙트 적용
	CommitAbility(Handle, ActorInfo, ActivationInfo);

	if (TriggerEventData->TargetData.Num() == 0)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	//공격자의 어트리뷰트셋을 가져온다.
	AActor* SourceActor = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	const UZombieBossAttributeSet* SourceAttri = SourceASC->GetSet<UZombieBossAttributeSet>();
	if (!SourceAttri)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	int32 Count = TriggerEventData->TargetData.Num();
	for (int32 i = 0; i < Count; ++i)
	{
		FGameplayAbilityTargetData_SingleTargetHit* HitData =
			static_cast<FGameplayAbilityTargetData_SingleTargetHit*>(TriggerEventData->TargetData.Data[i].Get());
		if (!HitData)
		{
			continue;
		}

		AActor* TargetActor = HitData->HitResult.GetActor();
		if (!IsValid(TargetActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
		if (!TargetASC)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}

		const UBaseAttributeSet* TargetAttri = TargetASC->GetSet<UBaseAttributeSet>();
		if (!TargetAttri)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}

		//Damage Effect를 발동시키기 위해 Spec Handle을 만든다.
		FGameplayEffectContextHandle Context = MakeEffectContext(Handle, ActorInfo);
		Context.AddHitResult(HitData->HitResult);

		AZombieBoss* ZombieActor = Cast<AZombieBoss>(SourceActor);
		TSubclassOf<UGameplayEffect> DamageEffect = ZombieActor->GetEffect(TEXT("Skill4Damage"));

		if (!IsValid(DamageEffect))
		{
			RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("Skill4Damage Effect를 지정하지 않았습니다."));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}

		FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
		DamageSpec.Data->SetContext(Context);

		//스킬 데미지를 계산한다.
		float Attack = SourceAttri->GetBoss_Skill4Damage();
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
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_ZombieBoss_Skill4::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//등록해 둔 쿨타임 이펙트를 가져온다.
	UGameplayEffect* CDEffect = GetCooldownGameplayEffect();

	if (CDEffect == nullptr)
	{
		return;
	}

	//좀비의 쿨타임 수치를 가져온다.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UZombieBossAttributeSet* Attri = ASC->GetSet<UZombieBossAttributeSet>();
	float CoolTime = Attri->GetBoss_Skill4Cooldown();

	//쿨타임 이펙트 스펙핸들을 만든다.
	FGameplayEffectSpecHandle CDSpec = MakeOutgoingGameplayEffectSpec(CDEffect->GetClass(), GetAbilityLevel());

	//스펙 핸들에 setbycaller로 넣어줄 값을 지정한다.
	CDSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.BZSkillCooldownDuration")), CoolTime);

	//나 자신에게 쿨타임 이펙트를 적용한다.
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CDSpec);
}



