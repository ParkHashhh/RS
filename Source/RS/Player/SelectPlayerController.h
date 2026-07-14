// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "SelectPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class RS_API ASelectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASelectPlayerController();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AActor> SelectCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> SelectWidgetClass;

	TObjectPtr<class USelectUserWidget> SelectWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	TArray<TObjectPtr<USkeletalMesh>> MaleMeshs;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	TArray<TObjectPtr<USkeletalMesh>> FeMaleMeshs;

	UPROPERTY()
	TObjectPtr<class ASelectPawn> CurrentSelectedPawn;

protected:
	virtual void SetupInputComponent() override;
	void HandleLeftClick();
public:
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	void OnPawnSelected(class ASelectPawn* Pawn);

public:
	UFUNCTION(Server, Reliable)
	void Server_SetSelectedCharacterInfo(FName TextureName, FName SkillName);
};
