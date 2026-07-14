// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerControllerBase.h"
#include "GameFramework/PlayerController.h"
#include "ColonyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RS_API AColonyPlayerController : public APlayerControllerBase
{
	GENERATED_BODY()

public:
	AColonyPlayerController();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> ColonyWidgetClass;

	TObjectPtr<UUserWidget> ColonyWidget;

public:
	TObjectPtr<UUserWidget> GetColonyWidget()
	{
		return ColonyWidget;
	}
protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(Server, Reliable)
	void Server_EnterStage();
};
