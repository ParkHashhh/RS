// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LoadingPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RS_API ALoadingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALoadingPlayerController();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	TObjectPtr<UUserWidget> LoadingWidget;

	FName LoadingLevelName;

protected:
	virtual void BeginPlay() override;
};
