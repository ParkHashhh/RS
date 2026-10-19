// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StartPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RS_API AStartPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AStartPlayerController();

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	bool TryStartSinglePlayer();   // true면 저장된 데이터로 이어하기 가능

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void StartMultiPlayer();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> StartWidgetClass;

	TObjectPtr<UUserWidget> StartWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> SettingWidgetClass;

protected:
	virtual void BeginPlay() override;

	void CreateSettingWidget();
};
