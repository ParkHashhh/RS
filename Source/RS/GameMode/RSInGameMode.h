// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RSInGameMode.generated.h"

/**
 *
 */
class ATravelPortal;

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

	UPROPERTY(EditDefaultsOnly, Category = "Portal")
	TSubclassOf<ATravelPortal> TravelPortalClass;

	UPROPERTY(EditDefaultsOnly, Category = "Portal")
	FTransform PortalSpawnTransform;

	UPROPERTY()
	TObjectPtr<ATravelPortal> ActivePortal;

public:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	void CreateOrUpdatePortal(FName LevelName, const FString& PackagePath);

protected:
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
};
