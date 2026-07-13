// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APlayerCharacterBase> MaleCharacterClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class APlayerCharacterBase> FeMaleCharacterClass;
public:
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole,
		const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	
};
