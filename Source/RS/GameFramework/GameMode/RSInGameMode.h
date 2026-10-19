// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GameFramework/GameMode/RSGameMode.h"
#include "RSInGameMode.generated.h"

class APlayerCharacterBase;
/**
 *
 */
class ATravelPortal;

UCLASS()
class RS_API ARSInGameMode : public ARSGameMode
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
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual bool MustSpectate_Implementation(APlayerController* NewPlayerController) const override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
protected:
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

};
