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
#include "PlayerCharacterBase.generated.h"

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
	TObjectPtr<class USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UInputData> InputDataClass;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPlayerAbilitySystemComponent> ASC;

	TObjectPtr<class UPlayerAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;


	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<FName, TSubclassOf<UGameplayEffect>> Effects;


	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> Abilitys;

	FGenericTeamId TeamID;

	TObjectPtr<class UPlayerAnimInstance> AnimInst;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsAttacking, meta=(AllowPrivateAccess=true))
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsDashing, meta=(AllowPrivateAccess=true))
	bool bIsDashing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsAiming, meta=(AllowPrivateAccess=true))
	bool bIsAiming = false;

	float CurrentArmLength; // 현재 타겟 길이
	float TargetArmLength; // 목표 길이

	// 기본 감도 값
	float MouseXSensitivity = 1.0f;
	float MouseYSensitivity = 1.0f;

	// 1m 거리에서 탄이 퍼지는 반경
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Combat|Shot")
	float ShotSpreadRadius;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_CurrentShotSpreadRadius, VisibleAnywhere, BlueprintReadOnly,
		meta=(AllowPrivateAccess=true), Category="Combat|Shot")
	float CurrentShotSpreadRadius;

	bool bASCReady = false;

public:
	FOnAttackingChangedDelegate OnAttackingChanged;
	FOnDashingChangedDelegate OnDashingChanged;
	FOnAimingChangedDelegate OnAimingChanged;
	FOnCurrentShotSpreadChangedDelegate OnCurrentShotSpreadChanged;

	FOnASCReadyDelegate OnASCReady;

public:
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
	void SetIsAiming(bool bNewIsAiming)
	{
		if (bIsAiming == bNewIsAiming)
		{
			return;
		}

			bIsAiming = bNewIsAiming;
			OnIsAimingChanged();
	}

	FORCEINLINE float GetCurrentShotSpreadRadius() const
	{
		return CurrentShotSpreadRadius;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Current Shot Spread Radius")
	float K2_GetCurrentShotSpreadRadius() const
	{
		return GetCurrentShotSpreadRadius();
	}

	UFUNCTION(BlueprintCallable)
	void SetCurrentShotSpreadRadius(float NewCurrentShotSpreadRadius)
	{
		if (CurrentShotSpreadRadius == NewCurrentShotSpreadRadius)
		{
			return;
		}

		if (HasAuthority())
		{
			CurrentShotSpreadRadius = NewCurrentShotSpreadRadius;
			OnCurrentShotSpreadRadiusChanged();
		}
		else
		{
			Server_SetCurrentShotSpreadRadius(NewCurrentShotSpreadRadius);
		}
	}

	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Combat")
	void Server_SetCurrentShotSpreadRadius(float NewCurrentShotSpreadRadius);

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

protected:
	UFUNCTION(Server, Reliable)
	void Server_SetMaxWalkSpeed(float NewSpeed);

	void OnIsAttackingChanged() const;
	void OnIsDashingChanged() const;
	void OnIsAimingChanged() const;

	UFUNCTION()
	void OnRep_CurrentShotSpreadRadius() const;
	void OnCurrentShotSpreadRadiusChanged() const;

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

	void AttatchWeapon();

	UFUNCTION()
	void LoadDefaultData();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void OnConstruction(const FTransform& Transform) override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE virtual UAbilitySystemComponent* GetASC() const override;

	void ShootAttackRay();

	TSubclassOf<UGameplayEffect> GetEffect(const FName TagName) const;

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	UFUNCTION(Server, Reliable)
	void Apply_Damage_Server(AActor* TargetActor, FHitResult Hit);

public:
	void PlayBaseAttackMontage();
};
