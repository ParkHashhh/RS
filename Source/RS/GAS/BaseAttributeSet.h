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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Attack, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Defense;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_HP, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	HP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	HPMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_WalkSpeed, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_RunSpeed, meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AttackDistance, Category = "Attribute", meta = (AllowPrivateAccess = true))
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	virtual void OnHpChanged(AActor* Instigator);
protected:

	UFUNCTION()
	virtual void OnRep_Attack(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_Defense(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_HP(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_WalkSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_RunSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_AttackDistance(const FGameplayAttributeData& OldValue);
};
