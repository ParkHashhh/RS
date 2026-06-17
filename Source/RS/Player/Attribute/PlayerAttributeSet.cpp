// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "RS/LogCategory.h"

void UPlayerAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerAttributeSet, AttackSpeed)
	DOREPLIFETIME(UPlayerAttributeSet, ShotSpreadRadius)
}

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
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
}

void UPlayerAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetShotSpreadRadiusAttribute())
	{
		//
		//
		//
		// (10.F, FColor::Red, TEXT("%s: 탄착군이 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, NewValue);
		// RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: 탄착군이 변경됨! 현재 값: %.2f"), *LOG_LINE_INFO, NewValue);
	}
}

void UPlayerAttributeSet::OnRep_AttackSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, AttackSpeed, OldValue);
}

void UPlayerAttributeSet::OnRep_ShotSpreadRadius(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UPlayerAttributeSet, ShotSpreadRadius, OldValue);
}

