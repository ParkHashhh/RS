// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ASCProvider.generated.h"

class UAbilitySystemComponent;
// This class does not need to be modified.
UINTERFACE()
class UASCProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class RS_API IASCProvider
{
	GENERATED_BODY()

public:
	virtual UAbilitySystemComponent* GetASC() const = 0;
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
};
