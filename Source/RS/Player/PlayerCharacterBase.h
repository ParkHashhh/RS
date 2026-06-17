// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "PlayerAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "RS/RSCharacter.h"
#include "RS/LogCategory.h"
#include "RS/Utils/ASCProvider.h"
#include "RS/Player/Character/PlayerAnimInstance.h"
#include "PlayerCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputData;
class UPlayerAttributeSet;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttackingChangedDelegate, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDashingChangedDelegate, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingChangedDelegate, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentShotSpreadChangedDelegate, float);

DECLARE_MULTICAST_DELEGATE(FOnASCReadyDelegate);

UCLASS()
class RS_API APlayerCharacterBase : public ARSCharacter,
                                    public IGenericTeamAgentInterface,
                                    public IAbilitySystemInterface,
                                    public IASCProvider
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInputData> InputDataClass;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPlayerAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPlayerAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<FName, TSubclassOf<UGameplayEffect>> Effects;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	FGenericTeamId TeamID;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UPlayerAnimInstance> AnimInst;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsAttacking, meta=(AllowPrivateAccess=true))
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsDashing, meta=(AllowPrivateAccess=true))
	bool bIsDashing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsAiming, meta=(AllowPrivateAccess=true))
	bool bIsAiming = false;

	bool bCanDash = false;

	bool bIsMoveKey = false;

	// 기본 감도 값
	float MouseXSensitivity = 1.0f;
	float MouseYSensitivity = 1.0f;

	bool bASCReady = false;

	// 기본값 세팅
	float TargetArmLength;
	FVector TargetCameraPos;
	FRotator TargetCameraRot;
	float TargetFOV;


public:
	FOnAttackingChangedDelegate OnAttackingChanged;
	FOnDashingChangedDelegate OnDashingChanged;
	FOnAimingChangedDelegate OnAimingChanged;
	FOnCurrentShotSpreadChangedDelegate OnCurrentShotSpreadChanged;

	FOnASCReadyDelegate OnASCReady;

public:
	bool GetIsMovekey()
	{
		return bIsMoveKey;
	}

	TObjectPtr<USkeletalMeshComponent> GetWeaponComponent()
	{
		return WeaponMeshComponent;
	}

	bool CanDash()
	{
		return !IsDashing() && !IsAiming() && !IsAttacking() && AnimInst && bCanDash;
	}

	FORCEINLINE bool IsAttacking() const
	{
		return bIsAttacking;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Attacking")
	bool K2_IsAttacking() const
	{
		return IsAttacking();
	}

	UFUNCTION(BlueprintCallable)
	void SetIsAttacking(bool bNewIsAttacking)
	{
		if (bIsAttacking == bNewIsAttacking)
		{
			return;
		}

		bIsAttacking = bNewIsAttacking;

		if (!IsAiming())
		{
			UpdatePlayerCameraState();
			UpdatePlayerSpeedState();
		}

		OnIsAttackingChanged();
	}

	FORCEINLINE bool IsDashing() const
	{
		return bIsDashing;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Dashing")
	bool K2_IsDashing() const
	{
		return IsDashing();
	}

	UFUNCTION(BlueprintCallable)
	void SetIsDashing(bool bNewIsDashing)
	{
		if (bIsDashing == bNewIsDashing)
		{
			return;
		}

		bIsDashing = bNewIsDashing;

		UpdatePlayerSpeedState();
		UpdatePlayerCameraState();
		OnIsDashingChanged();
	}

	FORCEINLINE bool IsAiming() const
	{
		return bIsAiming;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Aiming")
	bool K2_IsAiming() const
	{
		return IsAiming();
	}

	UFUNCTION(BlueprintCallable)
	void SetIsAiming(bool bNewIsAiming);

	FORCEINLINE UPlayerAttributeSet* GetAttributeSet()
	{
		return AttributeSet;
	}

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
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

	FORCEINLINE bool IsASCReady() const
	{
		return bASCReady;
	}

	void NotifyASCReady();

	void SetDefaultCameraPosition();
	void SetAimCameraPosition();
	void SetRunAndGunCameraPosition();
	void SetDashingCameraPosition();


protected:
	UFUNCTION(Server, Reliable)
	void Server_SetMaxWalkSpeed(float NewSpeed);

	void OnIsAttackingChanged() const;
	void OnIsDashingChanged() const;
	void OnIsAimingChanged() const;

private:
	// Input Function
	void MoveKey(const FInputActionValue& Value);
	void MoveReleaseKey(const FInputActionValue& Value);
	void DashKey(const FInputActionValue& Value);
	void DashReleaseKey(const FInputActionValue& Value);
	void AttackKey(const FInputActionValue& Value);
	void AttackReleaseKey(const FInputActionValue& Value);
	void JumpKey(const FInputActionValue& Value);
	void AimKey(const FInputActionValue& Value);
	void AimReleaseKey(const FInputActionValue& Value);
	void RotationMouse(const FInputActionValue& Value);
	// WeaponSkeleton Attach
	void AttachWeapon();

	void DebugKey0(const FInputActionValue& Value);
	void DebugKey1(const FInputActionValue& Value);
	void DebugKey2(const FInputActionValue& Value);
	void DebugKey3(const FInputActionValue& Value);
	void DebugKey4(const FInputActionValue& Value);
	void DebugKey5(const FInputActionValue& Value);
	void DebugKey6(const FInputActionValue& Value);
	void DebugKey7(const FInputActionValue& Value);
	void DebugKey8(const FInputActionValue& Value);
	void DebugKey9(const FInputActionValue& Value);

	void UpdatePlayerCameraState();
	void UpdatePlayerSpeedState();

	UFUNCTION()
	void LoadDefaultData();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE virtual UAbilitySystemComponent* GetASC() const override;

	void ShootAttackRay();

	TSubclassOf<UGameplayEffect> GetEffect(const FName TagName) const;
	void PlayBaseAttackMontage();


public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(AActor* TargetActor, FHitResult Hit);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="GAS|Abilities")
	TSubclassOf<UGameplayAbility> AimingAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="GAS|Effects")
	TSubclassOf<UGameplayEffect> AimingEffectClass;

private:
	FActiveGameplayEffectHandle AimingEffectHandle;

protected:
	virtual void GiveAbilities();
};
