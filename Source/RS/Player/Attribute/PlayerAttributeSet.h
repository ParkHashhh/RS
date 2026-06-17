// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "PlayerAttributeSet.generated.h"

/**
 *
 */
UCLASS()
class RS_API UPlayerAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_AttackSpeed, Category="Attribute", meta=(AllowPrivateAccess=true))
	FGameplayAttributeData	AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ShotSpreadRadius, Category="Attribute", meta=(AllowPrivateAccess=true))
	FGameplayAttributeData	ShotSpreadRadius;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, AttackSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, ShotSpreadRadius)


public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

protected:
	UFUNCTION()
	virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_ShotSpreadRadius(const FGameplayAttributeData& OldValue);
};
