// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "PlayerAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include <Components/TimelineComponent.h>
#include "RS/RSCharacter.h"
#include "RS/RSTags.h"
#include "RS/Player/Character/PlayerAnimInstance.h"
#include "RS/Utils/ASCProvider.h"
#include "PlayerCharacterBase.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputData;
class UPlayerAttributeSet;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDashingChangedDelegate, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentShotSpreadChangedDelegate, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimAlphaChangedDelegate, float);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Combat|Attack")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;

	FGenericTeamId TeamID;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UPlayerAnimInstance> AnimInst;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsDashing, meta=(AllowPrivateAccess=true))
	bool bIsDashing = false;

	bool bCanDash = false;

	bool bIsMoveKey = false;

	// 기본 감도 값
	float MouseXSensitivity = 1.0f;
	float MouseYSensitivity = 1.0f;

	bool bASCReady = false;

	// 기본값 세팅
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Default")
	float DefaultArmLength = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Default")
	FVector DefaultCameraPos = FVector(0.0, 50.0, 80.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Default")
	FRotator DefaultCameraRot = FRotator(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Default")
	float DefaultFOV = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Aim")
	float AimArmLength = 170.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Aim")
	FVector AimCameraPos = FVector(0.0, 60.0, 70.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Aim")
	FRotator AimCameraRot = FRotator(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Aim")
	float AimFOV = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|RunAndGun")
	float RunAndGunArmLength = 310.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|RunAndGun")
	FVector RunAndGunCameraPos = FVector(0.0, 80.0, 80.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|RunAndGun")
	FRotator RunAndGunCameraRot = FRotator(0.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|RunAndGun")
	float RunAndGunFOV = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Dash")
	float DashArmLength = 350.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Dash")
	FVector DashCameraPos = FVector(0.0, 80.0, 150.f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Dash")
	FRotator DashCameraRot = FRotator(-12.0f, 0.0f, 0.0f);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Dash")
	float DashFOV = 75.f;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Current")
	float TargetArmLength;
	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Current")
	FVector TargetCameraPos;
	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Current")
	FRotator TargetCameraRot;
	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Camera|Current")
	float TargetFOV;

public:
	FOnDashingChangedDelegate OnDashingChanged;
	FOnCurrentShotSpreadChangedDelegate OnCurrentShotSpreadChanged;

	FOnASCReadyDelegate OnASCReady;

public:
	TObjectPtr<UPlayerAnimInstance> GetAnimInst()
	{
		return AnimInst;
	}

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
		return ASC && ASC->HasMatchingGameplayTag(RSTags::Player::Attacking);
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Attacking")
	bool K2_IsAttacking() const
	{
		return IsAttacking();
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
		return ASC && ASC->HasMatchingGameplayTag(RSTags::Player::Aiming);
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Aiming")
	bool K2_IsAiming() const
	{
		return IsAiming();
	}

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

	TObjectPtr<UAnimMontage> GetBaseAttackMontage();

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(AActor* TargetActor, FHitResult Hit);

public:
	FOnAimAlphaChangedDelegate OnAimAlphaChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Combat|Aim")
	TSubclassOf<UGameplayAbility> AimingAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Combat|Aim")
	TObjectPtr<UCurveFloat> AimAlphaCurve;

private:
	FTimeline AimAlphaTimeline;

	UPROPERTY(Transient, BlueprintGetter=K2_GetAimAlpha, meta=(AllowPrivateAccess=true), Category="Combat|Aim")
	float AimAlpha;

public:
	FORCEINLINE float GetAimAlpha() const
	{
		return AimAlpha;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Aim Alpha")
	float K2_GetAimAlpha() const
	{
		return GetAimAlpha();
	}

protected:
	virtual void GiveAbilities();

private:
	UFUNCTION()
	void SetAimAlpha(float NewAimAlpha);

	void BindGameplayTagDelegate();

	void OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount);
};
