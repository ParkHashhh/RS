// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BaseAttributeSet.generated.h"

/**
 *
 */
UCLASS()
class RS_API UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Defense;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	HPMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	AttackDistance;


public:
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, Attack)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, Defense)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, HP)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, HPMax)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, WalkSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, RunSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, AttackDistance)


	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void OnHpChanged(AActor* Instigator);
};
