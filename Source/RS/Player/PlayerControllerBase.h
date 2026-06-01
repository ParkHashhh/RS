// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/RSPlayerController.h"
#include "PlayerControllerBase.generated.h"

/**
 * 
 */
UCLASS()
class RS_API APlayerControllerBase : public ARSPlayerController
{
	GENERATED_BODY()

public:
	APlayerControllerBase();


protected:
	FString			mPlayerName;


	FString GetPlayerName()	const
	{
		return mPlayerName;
	}

	void SetPlayerName(const FString& PlayerName)
	{
		mPlayerName = PlayerName;
	}
protected:
	virtual void BeginPlay();
	virtual void OnPossess(APawn* aPawn);
	virtual void OnUnPossess();
	virtual void Tick(float DeltaTime);

};
