// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffect.h"
#include "RS/LogCategory.h"

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* Instigator = nullptr;

	const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetContext();

	if (Context.IsValid())
	{
		Instigator = Context.GetOriginalInstigator();
		if (!IsValid(Instigator))
		{
			Instigator = Context.GetInstigator();
		}
	}

	if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		OnHpChanged(Instigator);
	}
}



void UBaseAttributeSet::OnHpChanged(AActor* Instigator)
{
}

void UBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseAttributeSet, Attack)
	DOREPLIFETIME(UBaseAttributeSet, Defense)
	DOREPLIFETIME(UBaseAttributeSet, HP)
	DOREPLIFETIME(UBaseAttributeSet, WalkSpeed)
	DOREPLIFETIME(UBaseAttributeSet, RunSpeed)
	DOREPLIFETIME(UBaseAttributeSet, AttackDistance)
}
void UBaseAttributeSet::OnRep_Attack(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, Attack, OldValue);

}

void UBaseAttributeSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, Defense, OldValue);

}

void UBaseAttributeSet::OnRep_HP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, HP, OldValue);

}

void UBaseAttributeSet::OnRep_WalkSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, WalkSpeed, OldValue);

}

void UBaseAttributeSet::OnRep_RunSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, RunSpeed, OldValue);

}

void UBaseAttributeSet::OnRep_AttackDistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, AttackDistance, OldValue);
}
