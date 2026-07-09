// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class RS_API IInteractableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnInteractionCompleted(AActor* Interactor) = 0;

	virtual float GetInteractionDuration() const
	{
		return 0.0f;
	}
};
