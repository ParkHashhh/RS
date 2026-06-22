// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZombieNormal.h"
#include "ZombieNormal_Crawler.generated.h"

/**
 * 
 */
UCLASS()
class RS_API AZombieNormal_Crawler : public AZombieNormal
{
	GENERATED_BODY()

public:
	AZombieNormal_Crawler();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Attack();
	
};
