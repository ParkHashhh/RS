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
	FGameplayAttributeData AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ShotSpreadRadius, Category="Attribute", meta=(AllowPrivateAccess=true))
	FGameplayAttributeData ShotSpreadRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentAmmo, Category="Attribute", meta=(AllowPrivateAccess=true))
	FGameplayAttributeData CurrentAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_MaxAmmo, Category="Attribute", meta=(AllowPrivateAccess=true))
	FGameplayAttributeData MaxAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_CriticalChance, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	CriticalChance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_CriticalDamage, Category="Attribute", meta=(AllowPrivateAccess = true))
	FGameplayAttributeData	CriticalDamage;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, AttackSpeed)
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, ShotSpreadRadius)
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, CurrentAmmo)
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, MaxAmmo)
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, CriticalChance)
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, CriticalDamage)

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void OnHPChanged(AActor* Instigator);
protected:
	UFUNCTION()
	virtual void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_ShotSpreadRadius(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CurrentAmmo(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_MaxAmmo(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalChance(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	virtual void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);
};
