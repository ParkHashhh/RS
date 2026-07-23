// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KillableInterface.generated.h"

class AZombieBase;
class APlayerCharacterBase;
// This class does not need to be modified.
UINTERFACE()
class UKillableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class RS_API IKillableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	void OnDead(const UWorld* World, const APawn* Instigator) const;
};
