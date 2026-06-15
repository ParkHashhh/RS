// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/RSPlayerController.h"
#include "PlayerControllerBase.generated.h"

class UBattleHUDWidget;
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
	FString PlayerName;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UBattleHUDWidget> BattleHUDWidgetClass;

public:
	FORCEINLINE FString GetPlayerName() const
	{
		return PlayerName;
	}

	void SetPlayerName(const FString& NewPlayerName)
	{
		PlayerName = NewPlayerName;
	}

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void CreateBattleHUD();

};
