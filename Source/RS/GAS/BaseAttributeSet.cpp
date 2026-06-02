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
		RS_LOG_DEBUG(LogRSBattle, Log, TEXT("%s: HP가 변경됨! 현재 HP: %.2f"), *LOG_LINE_INFO, GetHP());

		OnHpChanged(Instigator);
	}
}

void UBaseAttributeSet::OnHpChanged(AActor* Instigator)
{
}
