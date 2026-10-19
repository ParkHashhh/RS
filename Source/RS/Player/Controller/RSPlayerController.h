// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RSPlayerController.generated.h"

/**
 *
 */
UCLASS()
class RS_API ARSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ARSPlayerController();

public:
	virtual void OnRep_Pawn() override;
	virtual void OnRep_PlayerState() override;

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;

private:
	void NotifyPawnChanged(APawn* NewPawn);
};
