// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "Chest.generated.h"

class APlayerCharacterBase;
class UInteractableComponent;

UCLASS()
class RS_API AChest : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float OpenTime = 3.0f;

public:
	AChest();

	virtual void OnInteractionCompleted(AActor* Interactor) override;

	virtual float GetInteractionDuration() const override;

	const UStaticMeshComponent* GetMesh() const
	{
		return Mesh;
	}

	const UInteractableComponent* GetInteractable() const
	{
		return Interactable;
	}

	float GetOpenTime() const
	{
		return OpenTime;
	}

protected:
	virtual void BeginPlay() override;

private:
	void OnPlayerGetReward(APlayerCharacterBase* Player);
};
