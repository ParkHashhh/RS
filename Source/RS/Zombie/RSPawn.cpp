// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Zombie/RSPawn.h"


// Sets default values
ARSPawn::ARSPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARSPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARSPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARSPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

