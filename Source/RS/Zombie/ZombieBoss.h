// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZombieBase.h"
#include "ZombieBoss.generated.h"

class UZombieBossAttributeSet;

UCLASS()
class RS_API AZombieBoss : public AZombieBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AZombieBoss();

protected:
	UPROPERTY()
	TObjectPtr<UZombieBossAttributeSet> BossAttributeSet;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void Attack();
};
