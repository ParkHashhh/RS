// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RS/Interaction/InteractableInterface.h"
#include "HostOnlyChest.generated.h"

UCLASS()
class RS_API AHostOnlyChest : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHostOnlyChest();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnInteractionCompleted(AActor* Interactor) override;

	virtual bool CanInteract(APlayerCharacterBase* Interactor) override;
};
