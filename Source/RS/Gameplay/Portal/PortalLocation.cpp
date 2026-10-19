// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Gameplay/Portal/PortalLocation.h"


// Sets default values
APortalLocation::APortalLocation()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APortalLocation::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void APortalLocation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

