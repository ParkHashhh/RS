// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBossAttributeSet.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/Zombie/ZombieBoss.h"
#include "RS/LogCategory.h"
#include "GameplayEffectExtension.h"
#include "RS/Utils/KillableInterface.h"

UZombieBossAttributeSet::UZombieBossAttributeSet()
{

}

void UZombieBossAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	AActor* Instigator = nullptr;
	float Damage = 0.f;

	const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetContext();

	if (Context.IsValid())
	{
		Instigator = Context.GetOriginalInstigator();
		Damage = Data.EvaluatedData.Magnitude * -1.f;
		if (!IsValid(Instigator))
		{
			Instigator = Context.GetInstigator();
		}
	}

	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		OnHPChanged(Instigator, Damage);

		if (GetCurrentHP() <= 0.0f)
		{
			AActor* AvatarActor = GetOwningAbilitySystemComponent()->GetAvatarActor();
			if (const auto* Killable = Cast<IKillableInterface>(AvatarActor))
			{
				Killable->OnDead(GetWorld(), Cast<APawn>(Instigator));
			}
		}
	}
}

void UZombieBossAttributeSet::OnHPChanged(AActor* Instigator)
{
	//좀비의 HP변화를 처리하는 함수를 호출한다.
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (IsValid(ASC))
	{
		//GetOwnerActor와 GetAvatarActor의 차이점 체크
		AZombieBase* Zombie = Cast<AZombieBase>(ASC->GetAvatarActor());
		if (IsValid(Zombie))
		{
			Zombie->CallBackHP(Instigator);
		}
	}
}

void UZombieBossAttributeSet::OnHPChanged(AActor* Instigator, float Damage)
{
	//좀비의 HP변화를 처리하는 함수를 호출한다.
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (IsValid(ASC))
	{
		//GetOwnerActor와 GetAvatarActor의 차이점 체크
		AZombieBoss* Zombie = Cast<AZombieBoss>(ASC->GetAvatarActor());
		if (IsValid(Zombie))
		{
			Zombie->CallBackHP(Instigator, Damage);
		}
	}
}
