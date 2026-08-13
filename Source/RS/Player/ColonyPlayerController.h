// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerControllerBase.h"
#include "GameFramework/PlayerController.h"
#include "ColonyPlayerController.generated.h"

class UStageResultWidget;

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
	UPROPERTY()
	TObjectPtr<UUserWidget> ColonyWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MapSelectWidgetClass;
	UPROPERTY()

	TObjectPtr<UUserWidget> MapSelectWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UStageResultWidget> StageResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UStageResultWidget> ActiveStageResultWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Sound")
	TObjectPtr<USoundBase> MapSelectOpenSound;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Sound")
	TObjectPtr<USoundBase> MapSelectCloseSound;

public:
	TObjectPtr<UUserWidget> GetColonyWidget()
	{
		return ColonyWidget;
	}

	void ShowMapSelectWidget();
	void RemoveMapSelectWidget();
	void ShowStageResultWidget();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;

public:
	UFUNCTION(Server, Reliable)
	void Server_RequestPortal(FName LevelName, const FString& PackagePath);

	UFUNCTION(Client, Reliable)
	void Client_ShowStageResultWidget();

	UFUNCTION(Server, Reliable)
	void Server_ClearPendingStageResult();

	UFUNCTION(Server, Reliable)
	void Server_SetSelectedCharacterInfo(FName TextureName, FName SkillName);
};
