// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSCharacter.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "../RSConstants.h"
#include "AbilitySystemInterface.h"
#include "PlayerAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
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
	static inline FName CollisionProfileName = TEXT("Player");

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Body;

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
	TMap<FName,TSubclassOf<UGameplayEffect>> Effects;



	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> Abilitys;


	FGenericTeamId TeamID;

	UPROPERTY(Replicated)
	bool IsAttacking = false;

	bool IsDashing = false;
	bool IsAiming = false;

	float CurrentArmLength; // 현재 타겟 길이
	float TargetArmLength;  // 목표 길이

public:
	// Setter / Getter
	bool GetIsAttacking()
	{
		return IsAttacking;
	}

	UFUNCTION(Server, Reliable)
	void SetIsAttacking(bool Attacking);

	UPlayerAttributeSet* GetAttributeSet()
	{
		return AttributeSet;
	}
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const
	{
		return Cast<UAbilitySystemComponent>(ASC);
	}
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
protected:
	UFUNCTION(Server, Reliable)
	void Server_SetMaxWalkSpeed(float NewSpeed);

private:
	// Input Function
	void MoveKey(const FInputActionValue& Value);
	void DashKey(const FInputActionValue& Value);
	void DashReleaseKey(const FInputActionValue& Value);
	void AttackKey(const FInputActionValue& Value);
	void AttackReleaseKey(const FInputActionValue& Value);
	void SitKey(const FInputActionValue& Value);
	void AimKey(const FInputActionValue& Value);
	void AimReleaseKey(const FInputActionValue& Value);
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

public:
	void ShootAttackRay();

	TSubclassOf<UGameplayEffect> GetEffect(const FName TagName) const;

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID);
	virtual FGenericTeamId GetGenericTeamId() const;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;

protected:
	UFUNCTION(Server, Reliable)
	void Apply_Damage_Server(AActor* TargetActor, FHitResult Hit);
};
