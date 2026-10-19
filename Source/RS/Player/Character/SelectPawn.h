// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "SelectPawn.generated.h"

UCLASS()
class RS_API ASelectPawn : public APawn
{
	GENERATED_BODY()

public:
	ASelectPawn();
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> SelectMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mesh")
	TArray<TObjectPtr<USkeletalMesh>> SelectMeshes;

	UPROPERTY(EditInstanceOnly, Category = "Select")
	TObjectPtr<ACameraActor> ZoomCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skills")
	TArray<FName> SubSkillNames;

	FName CurrentSkillName;

	int32 CurrentMeshIndex = 0;

	int32 CurrentNameIndex = 0;
protected:
	virtual void BeginPlay() override;

public:
	TObjectPtr<ACameraActor> GetCamera()
	{
		return ZoomCamera;
	}

public:
	void NextTexture();
	void PrevTexture();

	void NextSkill();
	void PrevSkill();
	FName GetCurrentSkillName() const
	{
		return CurrentSkillName;
	}
	FName GetCurrentTextureName();

	USkeletalMesh* GetCurrentMesh()
	{
		return SelectMeshes[CurrentMeshIndex];
	}

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
