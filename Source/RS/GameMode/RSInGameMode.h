// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RSInGameMode.generated.h"

/**
 *
 */
UCLASS()
class RS_API ARSInGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARSInGameMode();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TSubclassOf<APlayerCharacterBase> MaleCharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TSubclassOf<APlayerCharacterBase> FeMaleCharacterClass;

public:

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

protected:
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
};
