// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "ZombieBossAttributeSet.generated.h"

/**
 *
 */
UCLASS()
class RS_API UZombieBossAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()

public:
	UZombieBossAttributeSet();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	DetectRange_Sight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	DetectRange_Hearing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill1Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill1Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill2Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill2Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill3Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill3Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill4Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill4Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill5Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill5Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill6Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_Skill6Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_MeleeAttackDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_MiddleAttackDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Boss_RangeAttackDistance;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, DetectRange_Sight)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, DetectRange_Hearing)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill1Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill1Cooldown)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill2Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill2Cooldown)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill3Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill3Cooldown)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill4Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill4Cooldown)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill5Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill5Cooldown)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill6Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_Skill6Cooldown)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_MeleeAttackDistance)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_MiddleAttackDistance)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieBossAttributeSet, Boss_RangeAttackDistance)

public:
	virtual void OnHPChanged(AActor* Instigator) override;
	virtual void OnHPChanged(AActor* Instigator, float Damage);
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
