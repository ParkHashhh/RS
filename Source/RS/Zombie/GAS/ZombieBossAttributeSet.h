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

public:
	virtual void OnHpChanged(AActor* Instigator) override;

};
