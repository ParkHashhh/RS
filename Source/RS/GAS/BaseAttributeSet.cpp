// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAttributeSet.h"

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UBaseAttributeSet::OnHpChanged(AActor* Instigator)
{
}
