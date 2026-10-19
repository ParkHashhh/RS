// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LoadingPlayerController.generated.h"

/**
 *
 */
class ULoadingUserWidget;

UCLASS()
class RS_API ALoadingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALoadingPlayerController();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<ULoadingUserWidget> LoadingWidgetClass;

	TObjectPtr<ULoadingUserWidget> LoadingWidget;

	FName NextLevelName;
	FString LoadingPackagePath;
	FName LoadingPackageCompleteName;
	bool bNextLevelIsListen;
	FString NextLevelOptions;
	bool bIsServerTravel;

	UPROPERTY()
	TObjectPtr<UPackage> LoadingPackage;

	float LoadingStartTime = 0.f;

	bool bLoadingStart = false;
	bool bLoadingPackage = false;
	bool bHasTraveled = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void StartAsyncLoading();
	void OnPackageLoading(const FName& PackageName, UPackage* LoadPackage, EAsyncLoadingResult::Type Result);
	void CompleteLoadingAndTravel();
};
