// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CreatureBase.h"
#include "BossCreatureBase.generated.h"

UCLASS()
class RS_API ABossCreatureBase : public ACreatureBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABossCreatureBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
