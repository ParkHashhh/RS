// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataTestActor.generated.h"

UCLASS()
class RS_API ADataTestActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADataTestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnItemInfoLoaded();
};
