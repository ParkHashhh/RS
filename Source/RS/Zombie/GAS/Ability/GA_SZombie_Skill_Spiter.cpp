// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_SZombie_Skill_Spiter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RS/Zombie/GAS/ZombieStandardAttributeSet.h"
#include "RS/Core/LogCategory.h"
#include "RS/Zombie/Special/ZombieSpecial.h"
#include "RS/Zombie/GAS/ZombieStandardAttributeSet.h"


UGA_SZombie_Skill_Spiter::UGA_SZombie_Skill_Spiter()
{

}


void UGA_SZombie_Skill_Spiter::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

	if (TriggerEventData->EventTag == FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.SZSkillLaunch")))
	{
		//발사 태그가 들어왔다면 쿨타임을 적용시키고 탈출
		CommitAbility(Handle, ActorInfo, ActivationInfo);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	else
	{
		//적중 태그가 들어왔다면 데미지, 독 이펙트 적용
		if (!TriggerEventData->Target)
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
		const UZombieStandardAttributeSet* SourceAttri = SourceASC->GetSet<UZombieStandardAttributeSet>();

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

		AZombieSpecial* ZombieActor = Cast<AZombieSpecial>(SourceActor);
		TSubclassOf<UGameplayEffect> DamageEffect = ZombieActor->GetEffect(TEXT("Damage"));

		if (!IsValid(DamageEffect))
		{
			RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("Damage Effect를 지정하지 않았습니다."));
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


		//독 이펙트 스펙 핸들을 만든다.
		TSubclassOf<UGameplayEffect> PoisonEffect = ZombieActor->GetEffect(TEXT("Poison"));
		if (!IsValid(PoisonEffect))
		{
			RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("Poison Effect를 지정하지 않았습니다."));
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
			return;
		}
		FGameplayEffectSpecHandle PoisonSpec = MakeOutgoingGameplayEffectSpec(PoisonEffect, GetAbilityLevel());
		PoisonSpec.Data->SetContext(Context);

		//독 데미지를 가져온다.
		float PoisonDamage = SourceAttri->GetSpecial_SkillDotDamage();
		PoisonSpec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.HP")), -PoisonDamage);

		//독 이펙트를 발동한다.
		ApplyGameplayEffectSpecToTarget(Handle, ActorInfo, ActivationInfo, PoisonSpec, TargetData);

		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
}

void UGA_SZombie_Skill_Spiter::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	//등록해 둔 쿨타임 이펙트를 가져온다.
	UGameplayEffect* CDEffect = GetCooldownGameplayEffect();

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
