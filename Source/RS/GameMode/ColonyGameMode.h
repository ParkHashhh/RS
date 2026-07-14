// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RSInGameMode.h"
#include "ColonyGameMode.generated.h"

/**
 *
 */
UCLASS()
class RS_API AColonyGameMode : public ARSInGameMode
{
	GENERATED_BODY()

public:
	AColonyGameMode();

protected:
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
		const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);
};
