// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "InputActionValue.h"
#include "PlayerAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include <Components/TimelineComponent.h>
#include "RS/RSCharacter.h"
#include "RS/RSTags.h"
#include "RS/RSUtils.h"
#include "RS/Player/Character/PlayerAnimInstance.h"
#include "RS/Utils/ASCProvider.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "PlayerCharacterBase.generated.h"

class UInteractionWidget;
class UInteractableComponent;
class UDataSubsystem;
class USpringArmComponent;
class UCameraComponent;
class UInputData;
class UPlayerAttributeSet;
class AGrenade;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnDashingChangedDelegate, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnCurrentShotSpreadChangedDelegate, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimAlphaChangedDelegate, float);

DECLARE_MULTICAST_DELEGATE(FOnASCReadyDelegate);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFocusedInteractableChanged, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHeldInteractableChanged, bool);

UCLASS()
class RS_API APlayerCharacterBase : public ARSCharacter,
                                    public IGenericTeamAgentInterface,
                                    public IAbilitySystemInterface,
                                    public IASCProvider
{
	GENERATED_BODY()
	friend class AGrenade;
	friend class AHealPack;
public:
	// Sets default values for this pawn's properties
	APlayerCharacterBase();

	static inline FName CapsuleComponentName = TEXT("CapsuleComponent");
	static inline FName CollisionProfileName = TEXT("Player");

protected:
	TObjectPtr<class APlayerControllerBase> PlayerController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> AIPerceptionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, Category = "VFX")
	TObjectPtr<UNiagaraSystem> BulletTrailEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInputData> InputDataClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UCameraShakeBase> HitShake;

	UPROPERTY()
	TObjectPtr<UCameraShakeBase> ActiveHitShakeInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPlayerAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UPlayerAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> BaseStatEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TMap<FName, TSubclassOf<UGameplayEffect>> Effects;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<class ABulletProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Actor")
	TSubclassOf<class AGrenade> GrenadeActor;

	UPROPERTY(EditDefaultsOnly, Category = "Actor")
	TSubclassOf<class AHealPack> HealPackActor;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Combat|Attack")
	TSubclassOf<UGameplayAbility> AttackAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Combat|Dash")
	TSubclassOf<UGameplayAbility> DashAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Combat|Reload")
	TSubclassOf<UGameplayAbility> ReloadAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "Combat|SubSkill")
	TSubclassOf<UGameplayAbility> SubSkillAbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "State|Dead")
	TSubclassOf<UGameplayAbility> DeadAbilityClass;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true), Category = "State|Movement")
	TSubclassOf<UGameplayAbility> JumpAbilityClass;

	FGenericTeamId TeamID;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UPlayerAnimInstance> AnimInst;



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsDashing, meta=(AllowPrivateAccess=true))
	bool bIsDashing = false;

	bool bCanDash = false;
	bool bCanJump = true;
	bool bIsAttackKeyPressed = false;

	bool bIsMoveKeyPressed = false;

	// 기본 감도 값
	float MouseXSensitivity = 1.0f;
	float MouseYSensitivity = 1.0f;

	bool bASCReady = false;

	UPROPERTY(Replicated)
	bool bIsDead = false;

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

	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilPitch = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilYawRange = 0.5f;

	// 보간 속도
	UPROPERTY(EditAnywhere, Category = "Recoil")
	float RecoilInterpSpeed = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Footstep")
	TMap<TObjectPtr<UPhysicalMaterial>, TObjectPtr<USoundBase>> FootstepSoundMap;

	// Tick에서 보간을 위한 변수
	float RecoilPitchCurrent = 0.f;
	float RecoilYawCurrent = 0.f;

	FOnASCReadyDelegate OnASCReady;

public:
	bool bCanRotate = true;
	bool bCanMove = true;

	UPROPERTY(Replicated)
	float ReplicatedViewPitch;

public:
	TObjectPtr<UCameraComponent> GetCameraComponent()
	{
		return Camera;
	}

	bool IsDead() const
	{
		return bIsDead;
	}
	UFUNCTION(BlueprintCallable, Category = "Camera")
	FRotator GetSpectateViewRotation() const;

	void MakeGrenade();
	void MakeHeal();
	void VisibleWeapon();
	void HideWeapon();

	bool IsAttackKeyPressed()
	{
		return bIsAttackKeyPressed;
	}

	void CallAttackAbility();
	template <typename T>
	void ExecuteOrBindOnASCReady(T* Object, void (T::*Callback)())
	{
		RSUtils::ExecuteOrBind(IsASCReady(), OnASCReady, Object, Callback);
	}

	template <typename T>
	void RemoveAllOnASCReady(T* Object)
	{
		static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "T must be derived from UObject");
		OnASCReady.RemoveAll(Object);
	}

	UPlayerAnimInstance* GetAnimInst()
	{
		return AnimInst;
	}

	bool IsMoveKeyPressed() const
	{
		return bIsMoveKeyPressed;
	}

	USkeletalMeshComponent* GetWeaponComponent()
	{
		return WeaponMeshComponent;
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
		return ASC && ASC->HasMatchingGameplayTag(RSTags::Player::Dashing);
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Dashing")
	bool K2_IsDashing() const
	{
		return IsDashing();
	}


	FORCEINLINE bool IsSubSkilling() const
	{
		return ASC && ASC->HasMatchingGameplayTag(RSTags::Player::SubSkilling);
	}

	UFUNCTION(BlueprintGetter, DisplayName = "Is SubSkilling")
	bool K2_IsSubSkilling() const
	{
		return IsSubSkilling();
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

	FORCEINLINE bool IsReloading() const
	{
		return ASC && ASC->HasMatchingGameplayTag(RSTags::Player::Reloading);
	}

	UFUNCTION(BlueprintGetter, DisplayName = "Is Reloading")
	bool K2_IsReloading() const
	{
		return IsReloading();
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

	UFUNCTION(Server, Reliable)
	void Server_SpawnGrenade(const FVector& SpawnLocation, const FRotator& GrenadeRotation);


	UFUNCTION(Server, Reliable)
	void Server_SpawnHeal(const FVector& SpawnLocation, const FRotator& HealRotation);


public:
	TSubclassOf<UGameplayAbility> GetDeadAbility()
	{
		return DeadAbilityClass;
	}
	void HitShakeCamera();
	void SetSkeletalMesh(FName TextureName);
	void SetSubSkillMontage(FName MontageName);

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
	void ReloadKey(const FInputActionValue& Value);
	void SubSkillKey(const FInputActionValue& Value);
	void SpecNextKey(const FInputActionValue& Value);
	void SpecPrevKey(const FInputActionValue& Value);
	void CursorKey(const FInputActionValue& Value);
	void CursorReleaseKey(const FInputActionValue& Value);
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

	void ApplyRecoil();
	void ShowEndPopUp();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	void ExecuteBulletTrailEffect(const FVector& MuzzleLocation, const FVector& TrailEnd);
	void ExecuteDecalEffect(const FHitResult& Hit);
	void ExecuteHitEffect(const FHitResult& Hit);

public:
	void ExecuteMuzzleEffect(const FVector& MuzzleLocation, const FRotator& MuzzleRotation);
	void PlayFootStepSound();
	void PlayMagazineRemove();
	void PlayMagazineInsert();
	void ReloadAmmo();
	bool UseAmmo();

public:
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
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
	TObjectPtr<UAnimMontage> GetReloadMontage();
	TObjectPtr<UAnimMontage> GetSubSkillMontage();
	TObjectPtr<UAnimMontage> GetDeadMontage();

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	UFUNCTION(Server, Reliable)
	void Server_ApplyDamage(AActor* TargetActor, FHitResult Hit,float Damage);

	UFUNCTION(Server, Reliable)
	void Server_ApplyHP(AActor* TargetActor, FHitResult Hit, float Hp);

	UFUNCTION(Server, Reliable)
	void Server_ShootAttackRay(float RandomX,float RandomY);

	bool CalculateHit(FHitResult& Hit, float RandomX, float RandomY);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ExecuteVisualEffect(const FHitResult& Hit, bool bIsHit);

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
	void OnAttackingTagChanged(const FGameplayTag Tag, int32 NewCount);
	void OnDashingTagChanged(const FGameplayTag Tag, int32 NewCount);
	void OnReloadingTagChanged(const FGameplayTag Tag, int32 NewCount);
	void OnSubSkillingTagChanged(const FGameplayTag Tag, int32 NewCount);
	void OnJumpingTagChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void OnPlayerDefaultInfoLoaded();

public:
	UFUNCTION()
	void OnDead();



#pragma region Inventory
private:
	void ShowInventory();
	void HideInventory();
	void ToggleInventory(bool bShow);
	void ToggleCombatWidgets(bool bShow);
#pragma endregion

#pragma region Interaction

public:
	FOnHeldInteractableChanged OnHeldInteractableChanged;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UInteractionWidget> InteractionWidget;

	UInteractionWidget* GetInteractionWidget();

	UPROPERTY(Transient)
	TWeakObjectPtr<UInteractableComponent> CurrentFocusedInteractable;

	UPROPERTY(Transient)
	TWeakObjectPtr<UInteractableComponent> CurrentHoldingInteractable;

	UPROPERTY()
	TArray<TWeakObjectPtr<UInteractableComponent>> OverlappedInteractables;

	FOnFocusedInteractableChanged OnFocusedInteractableChanged;

public:
	void RegisterOverlappedInteractable(UInteractableComponent* InComponent);
	void UnregisterOverlappedInteractable(UInteractableComponent* InComponent);

	UInteractableComponent* GetCurrentFocusedInteractable() const
	{
		if (UInteractableComponent* FocusingInteractable = CurrentFocusedInteractable.Get())
		{
			return FocusingInteractable;
		}

		if (UInteractableComponent* HoldingInteractable = CurrentHoldingInteractable.Get())
		{
			return HoldingInteractable;
		}

		return nullptr;
	}

private:
	void OnInteractPressed();
	void OnInteractReleased();

	UFUNCTION(Server, Reliable)
	void Server_StartInteraction(UInteractableComponent* TargetComp);

	UFUNCTION(Server, Reliable)
	void Server_CancelInteraction();

	void CheckInteractable();

	void HandleFocusedInteractableChange(bool bHasFocus);

	void HandleHeldInteractableChange(bool bHasHolding);

	UFUNCTION()
	void HandleInteractingTimeElapsed(float Elapsed, float Max);

#pragma endregion Interaction
};
