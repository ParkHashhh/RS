// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "RS/RSDefines.h"
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
	FGameplayAttributeData CurrHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHP;

public:
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, CurrHP)
	ATTRIBUTE_ACCESSORS(UBaseAttributeSet, MaxHP)

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void OnHpChanged(AActor* Instigator);
};
