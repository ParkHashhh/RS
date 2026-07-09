// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

#include "BaseAttributeSet.generated.h"


/**
 *
 */
UCLASS()
class RS_API UBaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_AttackPower, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_DefensePower, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentHP, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	CurrentHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHP, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_WalkSpeed, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_DashSpeed, Category="Attribute", meta=(AllowPrivateAccess=true))
	FGameplayAttributeData	RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_AttackDistance, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	AttackDistance;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, AttackPower)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, DefensePower)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, CurrentHP)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, MaxHP)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, WalkSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, RunSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UBaseAttributeSet, AttackDistance)


	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void OnHPChanged(AActor* Instigator);

protected:

	UFUNCTION()
	virtual void OnRep_AttackPower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_DefensePower(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CurrentHP(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxHP(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_WalkSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_DashSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_AttackDistance(const FGameplayAttributeData& OldValue);
};
