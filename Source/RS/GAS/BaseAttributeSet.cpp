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

	if (Data.EvaluatedData.Attribute == GetCurrentHPAttribute())
	{
		OnHPChanged(Instigator);
	}
}



void UBaseAttributeSet::OnHPChanged(AActor* Instigator)
{
}

void UBaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseAttributeSet, AttackPower)
	DOREPLIFETIME(UBaseAttributeSet, DefensePower)
	DOREPLIFETIME(UBaseAttributeSet, CurrentHP)
	DOREPLIFETIME(UBaseAttributeSet, WalkSpeed)
	DOREPLIFETIME(UBaseAttributeSet, RunSpeed)
	DOREPLIFETIME(UBaseAttributeSet, AttackDistance)
}

void UBaseAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UBaseAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHPAttribute())
	{
		if (GetOwningActor() && GetOwningActor()->HasAuthority())
		{
			// 최대 체력 변화에 따른 현재 체력 변화
			float Diff = NewValue - OldValue;
			// RS_LOG_DEBUG(LogRSGAS, Log, TEXT("%s: MaxHP Changed. Diff: %.2f"), *LOG_LINE_INFO, Diff);
			if (Diff > 0)
			{
				if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
				{
					float TargetCurrentHP = FMath::Clamp(GetCurrentHP() + Diff, 0.0f, NewValue);
					// RS_LOG_DEBUG(LogRSGAS, Log, TEXT("%s: MaxHP increased. Target Current HP: %.2f"), *LOG_LINE_INFO, TargetCurrentHP);
					ASC->SetNumericAttributeBase(GetCurrentHPAttribute(), TargetCurrentHP);
				}
			}
			else if (GetCurrentHP() < NewValue)
			{
				if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
				{
					// RS_LOG_DEBUG(LogRSGAS, Log, TEXT("%s: MaxHP decreased under current HP. Target Current HP: %.2f"), *LOG_LINE_INFO, NewValue);
					ASC->SetNumericAttributeBase(GetCurrentHPAttribute(), NewValue);
				}
			}
		}
	}
}

void UBaseAttributeSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, AttackPower, OldValue);

}

void UBaseAttributeSet::OnRep_DefensePower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, DefensePower, OldValue);

}

void UBaseAttributeSet::OnRep_CurrentHP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, CurrentHP, OldValue);

}

void UBaseAttributeSet::OnRep_MaxHP(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, MaxHP, OldValue);
}

void UBaseAttributeSet::OnRep_WalkSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, WalkSpeed, OldValue);

}

void UBaseAttributeSet::OnRep_DashSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, RunSpeed, OldValue);

}

void UBaseAttributeSet::OnRep_AttackDistance(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UBaseAttributeSet, AttackDistance, OldValue);
}
