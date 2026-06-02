// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffect.h"

void UBaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* Instigator = nullptr;

	const FGameplayEffectContextHandle& Context = Data.EffectSpec.GetContext();

	if (Context.IsValid())
	{
		Instigator = Context.GetOriginalInstigator();

		if (!Instigator)
			Instigator = Context.GetInstigator();
	}

	if (Data.EvaluatedData.Attribute == GetHPAttribute())
	{
		UE_LOG(LogTemp, Warning, TEXT("HP가 변경됨! 현재 HP: %f"), GetHP());
	}

}

void UBaseAttributeSet::OnHpChanged(AActor* Instigator)
{
}
