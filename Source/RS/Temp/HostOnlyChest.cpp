// Fill out your copyright notice in the Description page of Project Settings.


#include "HostOnlyChest.h"

#include "RS/Player/Character/PlayerCharacterBase.h"


// Sets default values
AHostOnlyChest::AHostOnlyChest()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AHostOnlyChest::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AHostOnlyChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHostOnlyChest::OnInteractionCompleted(AActor* Interactor)
{
}

bool AHostOnlyChest::CanInteract(APlayerCharacterBase* Interactor)
{
	if (!IInteractableInterface::CanInteract(Interactor))
	{
		return false;
	}

	if (!Interactor || !Interactor->HasAuthority())
	{
		return false;
	}

	return true;
}

