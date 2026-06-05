// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
#include "ZombieSpecial.generated.h"

class UZombieStandardAttributeSet;

UCLASS()
class RS_API AZombieSpecial : public AZombieBase
{
	GENERATED_BODY()

public:
	AZombieSpecial();

protected:
	UPROPERTY()
	TObjectPtr<UZombieStandardAttributeSet> SpecialAttributeSet;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Attack();
	
};
