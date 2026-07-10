// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "ZombieStandardAttributeSet.generated.h"

/**
 *
 */
UCLASS()
class RS_API UZombieStandardAttributeSet : public UBaseAttributeSet
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	DetectRange_Sight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	DetectRange_Hearing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Normal_Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Special_SkillCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Special_SkillDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FGameplayAttributeData	Special_SkillDotDamage;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UZombieStandardAttributeSet, DetectRange_Sight)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieStandardAttributeSet, DetectRange_Hearing)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieStandardAttributeSet, Normal_Rarity)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieStandardAttributeSet, Special_SkillCooldown)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieStandardAttributeSet, Special_SkillDamage)
	ATTRIBUTE_ACCESSORS_BASIC(UZombieStandardAttributeSet, Special_SkillDotDamage)
public:
	virtual void OnHPChanged(AActor* Instigator) override;

};
