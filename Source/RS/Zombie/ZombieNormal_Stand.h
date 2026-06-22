// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZombieNormal.h"
#include "ZombieNormal_Stand.generated.h"

/**
 * 
 */
UCLASS()
class RS_API AZombieNormal_Stand : public AZombieNormal
{
	GENERATED_BODY()

public:
	AZombieNormal_Stand();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Attack();
};
