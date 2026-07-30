// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_ZombieSpecial_Skill.h"
#include "RS/Zombie/GAS/ZombieStandardAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RS/Zombie/ZombieSpecial.h"
#include "RS/LogCategory.h"

UGA_ZombieSpecial_Skill::UGA_ZombieSpecial_Skill()
{

}

void UGA_ZombieSpecial_Skill::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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
	const UZombieStandardAttributeSet* SourceAttri = SourceASC->GetSet<UZombieStandardAttributeSet>();
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

		AZombieSpecial* ZombieActor = Cast<AZombieSpecial>(SourceActor);
		TSubclassOf<UGameplayEffect> DamageEffect = ZombieActor->GetEffect(TEXT("SkillDamage"));

		if (!IsValid(DamageEffect))
		{
			RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("SkillDamage Effect를 지정하지 않았습니다."));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}

		FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
		DamageSpec.Data->SetContext(Context);

		//스킬 데미지를 계산한다.
		float Attack = SourceAttri->GetSpecial_SkillDamage();
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

void UGA_ZombieSpecial_Skill::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//등록해 둔 쿨타임 이펙트를 가져온다.
	UGameplayEffect* CDEffect = GetCooldownGameplayEffect();

	//등록한 쿨타임 어빌리티가 없다면 그냥 넘어간다.
	if (!IsValid(CDEffect))
	{
		return;
	}

	//좀비의 쿨타임 수치를 가져온다.
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UZombieStandardAttributeSet* Attri = ASC->GetSet<UZombieStandardAttributeSet>();
	float CoolTime = Attri->GetSpecial_SkillCooldown();

	//쿨타임 이펙트 스펙핸들을 만든다.
	FGameplayEffectSpecHandle CDSpec = MakeOutgoingGameplayEffectSpec(CDEffect->GetClass(), GetAbilityLevel());

	//스펙 핸들에 setbycaller로 넣어줄 값을 지정한다.
	CDSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.SZSkillCooldown")), CoolTime);

	//나 자신에게 쿨타임 이펙트를 적용한다.
	ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, CDSpec);
}

void UGA_ZombieSpecial_Skill::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	//특수좀비를 얻어온다.
	AZombieSpecial* SourceActor = Cast<AZombieSpecial>(GetAvatarActorFromActorInfo());
	if (IsValid(SourceActor))
	{
		//어빌리티 종료 후처리 함수를 호출한다.
		SourceActor->OnEndAbility();
	}
}







