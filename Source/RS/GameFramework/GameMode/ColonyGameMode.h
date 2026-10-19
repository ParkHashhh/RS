// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GameFramework/GameMode/RSInGameMode.h"
#include "ColonyGameMode.generated.h"

class AColonyPlayerController;
/**
 *
 */
UCLASS()
class RS_API AColonyGameMode : public ARSInGameMode
{
	GENERATED_BODY()

public:
	AColonyGameMode();

	virtual void InitGameState() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	void OnPlayerCharacterSelected(AColonyPlayerController* PlayerController, FName TextureName, FName SkillName);

protected:
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
		const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage);
};
