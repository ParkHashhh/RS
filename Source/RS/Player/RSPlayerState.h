// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RS/Utils/AscProvider.h"
#include "RSPlayerState.generated.h"

class UAbilitySystemComponent;
/**
 *
 */
UCLASS()
class RS_API ARSPlayerState : public APlayerState, public IAscProvider
{
private:
	GENERATED_BODY()

public:
	ARSPlayerState();

public:
	virtual UAbilitySystemComponent* GetASC() const override;
};
