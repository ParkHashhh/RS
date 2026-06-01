// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSCharacter.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "PlayerCharacterBase.generated.h"

UCLASS()
class RS_API APlayerCharacterBase : public ARSCharacter,
	public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerCharacterBase();

	static inline FName CapsuleComponentName = TEXT("CapsuleComponent");

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UInputData> InputDataClass;

	// 기본 감도 값
	float MouseXSensitivity = 1.0f;
	float MouseYSensitivity = 1.0f;

public:
	// 🌟 미래의 UI나 State 클래스가 접근할 수 있도록 Getter / Setter 함수를 뚫어놓습니다.
	void SetMouseSensitivity(float NewX, float NewY)
	{
		MouseXSensitivity = NewX;
		MouseYSensitivity = NewY;
	}
	float GetMouseXSensitivity() const
	{
		return MouseXSensitivity;
	}
	float GetMouseYSensitivity() const
	{
		return MouseYSensitivity;
	}
private:
	void MoveKey(const FInputActionValue& Value);
	void DashKey(const FInputActionValue& Value);
	void AttackKey(const FInputActionValue& Value);
	void SitKey(const FInputActionValue& Value);

	void RotationMouse(const FInputActionValue& Value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void OnConstruction(const FTransform& Transform);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
