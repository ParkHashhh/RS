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

public:
	ATTRIBUTE_ACCESSORS_BASIC(UZombieStandardAttributeSet, DetectRange_Sight)

public:
	virtual void OnHpChanged(AActor* Instigator);
	
};
