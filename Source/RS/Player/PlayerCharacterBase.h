// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSCharacter.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "AbilitySystemInterface.h"
#include "PlayerAbilitySystemComponent.h"
#include "PlayerCharacterBase.generated.h"


UCLASS()
class RS_API APlayerCharacterBase : public ARSCharacter,
	public IGenericTeamAgentInterface,
	public IAbilitySystemInterface
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPlayerAbilitySystemComponent> ASC;

	TObjectPtr<class UPlayerAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;


	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> Abilitys;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> AttackCooldownEffectClass;
public:
	// IAbilitySystemInterface를 구현하기 위한 필수 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const
	{
		return Cast<UAbilitySystemComponent>(ASC);
	}

public:
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

protected:

	UFUNCTION(Server, Reliable)
	void Attack_Server();
	void Attack_Server_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Attack_Execution();	
	void Attack_Execution_Implementation();

public:
	UFUNCTION(Server, Reliable)
	virtual void NormalAttack_Server();
	virtual void NormalAttack_Server_Implementation();
};
