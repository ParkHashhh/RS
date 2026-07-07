// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "RSPlayerState.h"
#include "Attribute/PlayerAttributeSet.h"
#include "PlayerControllerBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Input/InputData.h"
#include "RS/Zombie/ZombieBase.h"
#include "Actor/BulletProjectile.h"
#include "Actor/Grenade.h"
#include "Actor/HealPack.h"
#include "GameplayCueManager.h"
#include "PlayerControllerBase.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/UI/Battle/BattleHUDWidget.h"
#include "RS/UI/Battle/Inventory/Popup/InventoryPopupWidget.h"
#include "RS/UI/Subsystem/UISubsystem.h"

// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	// SpringArm의 부모 컴포넌트로 Capsule을 지정한다.
	SpringArm->SetupAttachment(GetCapsuleComponent());

	GetMesh()->AddRelativeLocation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

	SpringArm->SetRelativeRotation(FRotator::ZeroRotator);

	// SpringArm Roation은 PawnRotation을 따라는것이 아닌 직접관리. -> 관전자 시점 때문에 필요
	SpringArm->bUsePawnControlRotation = false;

	// CameraComponent를 생성하고 SpringArm에 붙여준다.
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = true;

	USkeletalMeshComponent* PlayerMesh = GetMesh();

	if (IsValid(PlayerMesh))
	{
		PlayerMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UCapsuleComponent* PlayerBody = GetCapsuleComponent();

	if (IsValid(PlayerBody))
	{
		PlayerBody->SetCollisionProfileName(CollisionProfileName);
	}
	ASC = CreateDefaultSubobject<UPlayerAbilitySystemComponent>(TEXT("ASC"));

	AttributeSet = CreateDefaultSubobject<UPlayerAttributeSet>(TEXT("AttributeSet"));
	// Attribute를 가지고있는다.
	ASC->AddAttributeSetSubobject<UPlayerAttributeSet>(AttributeSet);

	ASC->SetIsReplicated(true);

	if (IsValid(SkeletalMeshAsset))
	{
		GetMesh()->SetSkeletalMeshAsset(SkeletalMeshAsset);
	}

	SetReplicates(true);
	SetReplicateMovement(true);

	SetGenericTeamId(FGenericTeamId(TeamPlayer));
}


// Called when the game starts or when spawned
void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	const UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);

	auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	DataSubsystem->ExecuteOrBindOnPlayerDefaultInfoLoaded(this, &ThisClass::LoadDefaultData);

	PlayerController = Cast<APlayerControllerBase>(GetController());

	if (IsValid(PlayerController))
	{
		UEnhancedInputLocalPlayerSubsystem* SubSystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		// BP_PlayerCharacterBase에 등록되어있는 InputDataClass를 가져와 Mapping한다.
		if (InputDataClass && SubSystem)
		{
			const UInputData* InputData = GetDefault<UInputData>(InputDataClass);
			if (InputData && InputData->Context)
			{
				SubSystem->AddMappingContext(InputData->Context, 0);
			}
		}
		FInputModeGameOnly InputModeData;

		InputModeData.SetConsumeCaptureMouseDown(true);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->bShowMouseCursor = false; // 마우스 커서 안 보이게 끄기

		if (PlayerController->PlayerCameraManager)
		{
			PlayerController->PlayerCameraManager->ViewPitchMin = -40.f;
			PlayerController->PlayerCameraManager->ViewPitchMax = 40.f;
		}
	}

	SetMouseSensitivity(0.7f, 0.7f);
	AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	UpdatePlayerSpeedState();
	UpdatePlayerCameraState();
	AttachWeapon();
	WeaponMeshComponent->SetIsReplicated(true);
	if (AimAlphaCurve)
	{
		FOnTimelineFloat OnTimelineFloat;
		OnTimelineFloat.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(APlayerCharacterBase, SetAimAlpha));
		AimAlphaTimeline.AddInterpFloat(AimAlphaCurve, OnTimelineFloat);
	}

	ExecuteOrBindOnASCReady(this, &ThisClass::BindGameplayTagDelegate);
}

void APlayerCharacterBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (GetCharacterMovement()->IsWalking())
	{
		bCanJump = true;

	}
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PlayerController = Cast<APlayerControllerBase>(NewController);

	if (IsValid(ASC))
	{
		ASC->InitAbilityActorInfo(this, this);

		if (HasAuthority())
		{
			GiveAbilities();

			if (UGameInstance* GI = GetGameInstance())
			{
				if (auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>())
				{
					DataSubsystem->ExecuteOrBindOnPlayerDefaultInfoLoaded(this, &ThisClass::OnPlayerDefaultInfoLoaded);
				}
			}
		}

		BindGameplayTagDelegate();
	}
}

void APlayerCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsValid(ASC))
	{
		ASC->InitAbilityActorInfo(this, this);

		if (UGameInstance* GI = GetGameInstance())
		{
			if (auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>())
			{
				DataSubsystem->ExecuteOrBindOnPlayerDefaultInfoLoaded(this, &ThisClass::OnPlayerDefaultInfoLoaded);
			}
		}

		BindGameplayTagDelegate();
	}
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacterBase, bIsDead);
	DOREPLIFETIME(APlayerCharacterBase, ReplicatedViewPitch);
}

UAbilitySystemComponent* APlayerCharacterBase::GetASC() const
{
	return GetAbilitySystemComponent();
}

// Called every frame
void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FRotator CurrentControlRot = GetControlRotation();
	FRotator ActorRot = GetActorRotation();

	float RelativePitch = FRotator::NormalizeAxis(CurrentControlRot.Pitch - ActorRot.Pitch);
	float ClampedPitch = FMath::Clamp(RelativePitch, -40.0f, 40.0f);
	// 서버에서 ViewPitch 호출하기전에 미리 클라이언트에서 자신의 화면부터 이동
	if (IsLocallyControlled())
	{
		ReplicatedViewPitch = ClampedPitch;
	}
	if (HasAuthority())
	{
		ReplicatedViewPitch = ClampedPitch;
	}



	float InterpSpeed = 10.f;
	// 현재의 위치에서 타겟위치까지 보간
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaTime, InterpSpeed);
	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, InterpSpeed);
	SpringArm->SetRelativeLocation(FMath::VInterpTo(SpringArm->GetRelativeLocation(), TargetCameraPos, DeltaTime,
	                                                InterpSpeed));
	Camera->SetRelativeRotation(
		FMath::RInterpTo(Camera->GetRelativeRotation(), TargetCameraRot, DeltaTime, InterpSpeed));

	if (IsLocallyControlled())
	{
		// 내마우스에따라 로테이션 설정
		SpringArm->SetWorldRotation(GetControlRotation());
	}
	else
	{
		// bUseControllerRotationPitch 은 False 이기때문에 직접 Replication한 Pitch값과 컨트롤러의 Yaw 값으로 설정한다.
		SpringArm->SetWorldRotation(GetSpectateViewRotation());
	}

	if (IsLocallyControlled())
	{
		if (AimAlphaTimeline.IsPlaying())
		{
			AimAlphaTimeline.TickTimeline(DeltaTime);
		}
	}

	if (!FMath::IsNearlyZero(RecoilPitchCurrent))
	{
		float PitchDelta = FMath::FInterpTo(0.f, RecoilPitchCurrent, DeltaTime, RecoilInterpSpeed);
		AddControllerPitchInput(-PitchDelta);
		RecoilPitchCurrent -= PitchDelta;
	}

	if (!FMath::IsNearlyZero(RecoilYawCurrent))
	{
		float YawDelta = FMath::FInterpTo(0.f, RecoilYawCurrent, DeltaTime, RecoilInterpSpeed);
		AddControllerYawInput(YawDelta);
		RecoilYawCurrent -= YawDelta;
	}
}

// Called to bind functionality to input
void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		return;
	}

	if (!InputDataClass)
	{
		return;
	}

	const UInputData* InputData = GetDefault<UInputData>(InputDataClass);

	if (!InputData)
	{
		return;
	}

	// FName으로 등록된 InputAction을 반환해 등록한다.
	UInputAction* Action = InputData->FindAction(FInputKeyNames::Move);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::MoveKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &APlayerCharacterBase::MoveReleaseKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Canceled, this, &APlayerCharacterBase::MoveReleaseKey);
	}
	else
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: MoveIAKey is not connected"), *LOG_LINE_INFO)
	}

	Action = InputData->FindAction(FInputKeyNames::Dash);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::DashKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &APlayerCharacterBase::DashReleaseKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Canceled, this, &APlayerCharacterBase::DashReleaseKey);
	}

	Action = InputData->FindAction(FInputKeyNames::Attack);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::AttackKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &APlayerCharacterBase::AttackReleaseKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Canceled, this, &APlayerCharacterBase::AttackReleaseKey);
	}

	Action = InputData->FindAction(FInputKeyNames::Jump);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::JumpKey);
	}

	Action = InputData->FindAction(FInputKeyNames::Rotation);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::RotationMouse);
	}

	Action = InputData->FindAction(FInputKeyNames::Aim);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::AimKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &APlayerCharacterBase::AimReleaseKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Canceled, this, &APlayerCharacterBase::AimReleaseKey);
	}

	Action = InputData->FindAction(FInputKeyNames::Reload);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::ReloadKey);
	}

	Action = InputData->FindAction(FInputKeyNames::SubSkill);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::SubSkillKey);
	}

	Action = InputData->FindAction(FInputKeyNames::SpecNext);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::SpecNextKey);
	}
	Action = InputData->FindAction(FInputKeyNames::SpecPrev);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::SpecPrevKey);
	}


	Action = InputData->FindAction(FInputKeyNames::Inventory);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::ShowInventory);
		EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &APlayerCharacterBase::HideInventory);
	}

#if !UE_BUILD_SHIPPING
	Action = InputData->FindAction(FInputKeyNames::Debug_00);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey0);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_01);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey1);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_02);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey2);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_03);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey3);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_04);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey4);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_05);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey5);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_06);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey6);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_07);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey7);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_08);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey8);
	}

	Action = InputData->FindAction(FInputKeyNames::Debug_09);
	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::DebugKey9);
	}
#endif
}

void APlayerCharacterBase::Server_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (IsValid(MovementComponent))
	{
		MovementComponent->MaxWalkSpeed = NewSpeed;
	}
}

void APlayerCharacterBase::NotifyASCReady()
{
	if (bASCReady)
	{
		return;
	}

	bASCReady = true;

	OnASCReady.Broadcast();
}


TObjectPtr<UAnimMontage> APlayerCharacterBase::GetBaseAttackMontage()
{
	return AnimInst->GetBaseAttackMontage();
}

TObjectPtr<UAnimMontage> APlayerCharacterBase::GetReloadMontage()
{
	return AnimInst->GetReloadMontage();
}

TObjectPtr<UAnimMontage> APlayerCharacterBase::GetSubSkillMontage()
{
	return AnimInst->GetSubSkillMontage();
}

TObjectPtr<UAnimMontage> APlayerCharacterBase::GetDeadMontage()
{
	return AnimInst->GetDeadMontage();
}

void APlayerCharacterBase::MoveKey(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (!bCanMove)
		return;

	AddMovementInput(GetActorForwardVector(), MovementVector.X);

	AddMovementInput(GetActorRightVector(), MovementVector.Y);

	bIsMoveKeyPressed = true;

	bCanDash = MovementVector.X > 0.9f && FMath::Abs(MovementVector.Y) < 0.1f;

	if (IsDashing())
	{
		if (!bCanDash)
		{
			DashReleaseKey(Value);
		}
	}
}

void APlayerCharacterBase::MoveReleaseKey(const FInputActionValue& Value)
{
	bIsMoveKeyPressed = false;
}

void APlayerCharacterBase::DashKey(const FInputActionValue& Value)
{
	if (bCanDash)
	{
		ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Dash));
	}
}

void APlayerCharacterBase::DashReleaseKey(const FInputActionValue& Value)
{
	ASC->AbilityLocalInputReleased(static_cast<int32>(ERSInputID::Dash));
}

void APlayerCharacterBase::AttackKey(const FInputActionValue& Value)
{
	// Bool 변수로 누르고 있는지 땠는지를 판단 한 후 공격속도마다 공격 함수를 호출하게 하고(이펙트 파티클 등등), 서버에서는 충돌처리 및 데미지 체크하기 => 완료
	if (!IsValid(ASC))
	{
		return;
	}
	ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Fire));
	bIsAttackKeyPressed = true;
}


void APlayerCharacterBase::AttackReleaseKey(const FInputActionValue& Value)
{
	ASC->AbilityLocalInputReleased(static_cast<int32>(ERSInputID::Fire));
	bIsAttackKeyPressed = false;
}


void APlayerCharacterBase::JumpKey(const FInputActionValue& Value)
{
	bool bJump = Value.Get<bool>();
	if (bJump)
	{
		if (bCanJump)
		{
			ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Jump));
		}
	}
}

void APlayerCharacterBase::AimKey(const FInputActionValue& Value)
{
	if (!IsValid(ASC) || !GetMovementComponent() )
	{
		return;
	}

	ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Aim));
}

void APlayerCharacterBase::AimReleaseKey(const FInputActionValue& Value)
{
	if (!ASC)
	{
		return;
	}

	ASC->AbilityLocalInputReleased(static_cast<int32>(ERSInputID::Aim));
}

void APlayerCharacterBase::ReloadKey(const FInputActionValue& Value)
{
	ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Reload));
}

void APlayerCharacterBase::SubSkillKey(const FInputActionValue& Value)
{
	ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::SubSkill));
}

void APlayerCharacterBase::SpecNextKey(const FInputActionValue& Value)
{
	if (IsValid(PlayerController))
	{
		PlayerController->SpectateNext();
	}
}

void APlayerCharacterBase::SpecPrevKey(const FInputActionValue& Value)
{
	if (IsValid(PlayerController))
	{
		PlayerController->SpectatePrev();
	}
}


void APlayerCharacterBase::Server_SpawnGrenade_Implementation(const FVector& SpawnLocation, const FRotator& GrenadeRotation)
{
	if (GrenadeActor)
	{

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// 수류탄 생성
		AGrenade* Grenade = GetWorld()->SpawnActorDeferred<AGrenade>(
			GrenadeActor, FTransform(GrenadeRotation, SpawnLocation), this, this,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (Grenade)
		{
			// 보는 방향으로 초기 속도 설정
			Grenade->LaunchDir = GrenadeRotation.Vector();
			Grenade->FinishSpawning(FTransform(GrenadeRotation, SpawnLocation));
		}
	}
}

void APlayerCharacterBase::Server_SpawnHeal_Implementation(const FVector& SpawnLocation, const FRotator& HealRotation)
{
	if (HealPackActor)
	{

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FRotator SpawnRotation = FRotator(0.f, HealRotation.Yaw, 0.f);

		AHealPack* HealPack = GetWorld()->SpawnActorDeferred<AHealPack>(
			HealPackActor, FTransform(SpawnRotation, SpawnLocation), this, this,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (HealPack)
		{
			HealPack->LaunchDir = HealRotation.Vector();
			HealPack->FinishSpawning(FTransform(SpawnRotation, SpawnLocation));
		}
	}
}


FRotator APlayerCharacterBase::GetSpectateViewRotation() const
{
	return FRotator(ReplicatedViewPitch, GetActorRotation().Yaw,0.f);
}

void APlayerCharacterBase::MakeGrenade()
{
	if (!IsLocallyControlled())
	{
		return; // 서버에서 노티파이로 호출된 경우 무시 (RPC로 처리되니까)
	}

	if (!IsValid(PlayerController))
	{
		return;
	}

	FHitResult Hit;

	FVector CameraLocation;
	FRotator CameraRotation;


	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// 임시로 5000
	FVector CameraEnd = CameraLocation + CameraRotation.Vector() * 5000.f;

	FHitResult CameraHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector TargetPoint = CameraEnd;
	if (GetWorld()->LineTraceSingleByChannel(CameraHit, CameraLocation, CameraEnd, ECC_Visibility, Params))
	{
		TargetPoint = CameraHit.ImpactPoint;
	}

	FVector SpawnLocation = GetMesh()->GetSocketLocation(FName("SubSkill_Socket"));
	FVector LaunchDirection = (TargetPoint - SpawnLocation).GetSafeNormal();

	FRotator LaunchRotation = LaunchDirection.Rotation();
	LaunchRotation.Pitch += 8.f;

	Server_SpawnGrenade(SpawnLocation, LaunchRotation);
}

void APlayerCharacterBase::MakeHeal()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!IsValid(PlayerController))
	{
		return;
	}
	FHitResult Hit;

	FVector CameraLocation;
	FRotator CameraRotation;


	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector CameraEnd = CameraLocation + CameraRotation.Vector() * 5000.f;

	FHitResult CameraHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FVector TargetPoint = CameraEnd;
	if (GetWorld()->LineTraceSingleByChannel(CameraHit, CameraLocation, CameraEnd, ECC_Visibility, Params))
	{
		TargetPoint = CameraHit.ImpactPoint;
	}

	FVector SpawnLocation = GetMesh()->GetSocketLocation(FName("SubSkill_Socket"));
	FVector LaunchDirection = (TargetPoint - SpawnLocation).GetSafeNormal();

	FRotator LaunchRotation = LaunchDirection.Rotation();
	LaunchRotation.Pitch += 12.f;
	Server_SpawnHeal(SpawnLocation, LaunchRotation);
}

void APlayerCharacterBase::VisibleWeapon()
{

	WeaponMeshComponent->SetVisibility(true, true);
	WeaponMeshComponent->SetHiddenInGame(false);

	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void APlayerCharacterBase::HideWeapon()
{
	WeaponMeshComponent->SetVisibility(false, true);
	WeaponMeshComponent->SetHiddenInGame(true);

	WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APlayerCharacterBase::CallAttackAbility()
{
	if (!IsValid(ASC))
	{
		return;
	}
	ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Fire));
}


void APlayerCharacterBase::RotationMouse(const FInputActionValue& Value)
{
	if (!bCanRotate)
		return;
	FVector Axis = Value.Get<FVector>();
	AddControllerYawInput(Axis.X * MouseXSensitivity);
	AddControllerPitchInput(Axis.Y * MouseYSensitivity);
}


void APlayerCharacterBase::SetDefaultCameraPosition()
{
	TargetArmLength = DefaultArmLength;
	TargetFOV = DefaultFOV;
	TargetCameraPos = DefaultCameraPos;
	TargetCameraRot = DefaultCameraRot;
}

void APlayerCharacterBase::SetAimCameraPosition()
{
	TargetArmLength = AimArmLength;
	TargetFOV = AimFOV;
	TargetCameraPos = AimCameraPos;
	TargetCameraRot = AimCameraRot;
}

void APlayerCharacterBase::SetRunAndGunCameraPosition()
{
	TargetArmLength = RunAndGunArmLength;
	TargetFOV = RunAndGunFOV;
	TargetCameraPos = RunAndGunCameraPos;
	TargetCameraRot = RunAndGunCameraRot;
}

void APlayerCharacterBase::SetDashingCameraPosition()
{
	TargetArmLength = DashArmLength;
	TargetFOV = DashFOV;
	TargetCameraPos = DashCameraPos;
	TargetCameraRot = DashCameraRot;
}

void APlayerCharacterBase::DebugKey0(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey0"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey0"));

	if (auto* PS = GetPlayerState<ARSPlayerState>())
	{
		if (UInventoryComponent* Inventory = PS->GetInventoryComponent())
		{
			Inventory->AddItem(FName("critical_damage"), 1);
		}
	}
}

void APlayerCharacterBase::DebugKey1(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey1"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey1"));

	if (auto* PS = GetPlayerState<ARSPlayerState>())
	{
		if (UInventoryComponent* Inventory = PS->GetInventoryComponent())
		{
			Inventory->AddItem(FName("bullet_recycle"), 1);
		}
	}
}

void APlayerCharacterBase::DebugKey2(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey2"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey2"));
	if (auto* PS = GetPlayerState<ARSPlayerState>())
	{
		if (UInventoryComponent* Inventory = PS->GetInventoryComponent())
		{
			Inventory->AddItem(FName("attack_speed"), 1);
		}
	}
}

void APlayerCharacterBase::DebugKey3(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey3"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey3"));

	if (auto* PS = GetPlayerState<ARSPlayerState>())
	{
		if (UInventoryComponent* Inventory = PS->GetInventoryComponent())
		{
			Inventory->AddItem(FName("ignite"), 1);
		}
	}
}

void APlayerCharacterBase::DebugKey4(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey4"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey4"));

	if (auto* PS = GetPlayerState<ARSPlayerState>())
	{
		if (UInventoryComponent* Inventory = PS->GetInventoryComponent())
		{
			Inventory->AddItem(FName("hp_booster"), 1);
		}
	}
}

void APlayerCharacterBase::DebugKey5(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey5"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey5"));

	if (AttributeSet)
	{
		float MaxAmmo = AttributeSet->GetMaxAmmo();
		AttributeSet->SetCurrentAmmo(MaxAmmo);
		RS_LOG_DEBUG(LogRSCommon, Warning, TEXT("%s: Debug reload ammo. curr ammo: %.0f"), *LOG_LINE_INFO, AttributeSet->GetCurrentAmmo());
	}
}

void APlayerCharacterBase::DebugKey6(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey6"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey6"));
}

void APlayerCharacterBase::DebugKey7(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey7"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey7"));
}

void APlayerCharacterBase::DebugKey8(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey8"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey8"));
}

void APlayerCharacterBase::DebugKey9(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey9"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey9"));
}

void APlayerCharacterBase::AttachWeapon()
{
	if (WeaponMeshComponent)
	{
		FName SocketName = FName(TEXT("Weapon_Socket"));
		WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
		                                       SocketName);
	}
}

void APlayerCharacterBase::UpdatePlayerCameraState()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	// 1. 공중
	if (!bCanJump)
	{
		SetDefaultCameraPosition();
		return;
	}
	// 2. 카메라 위치 결정
	if (IsAiming())
	{
		SetAimCameraPosition();
	}
	else if (IsAttacking())
	{
		SetRunAndGunCameraPosition();
	}
	else if (IsDashing())
	{
		SetDashingCameraPosition();
	}
	else
	{
		SetDefaultCameraPosition();
	}
}

void APlayerCharacterBase::UpdatePlayerSpeedState()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	float TargetSpeed = 400.f;

	if (!bCanJump)
	{
		MovementComponent->MaxWalkSpeed = TargetSpeed;
		Server_SetMaxWalkSpeed(TargetSpeed);
		return;
	}


	if (IsDashing())
	{
		TargetSpeed = 600.f;
	}
	else if (IsAiming())
	{
		TargetSpeed = 200.f;
	}
	else
	{
		TargetSpeed = 400.f;
	}
	MovementComponent->MaxWalkSpeed = TargetSpeed;
	Server_SetMaxWalkSpeed(TargetSpeed);
}

void APlayerCharacterBase::LoadDefaultData()
{
	const UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);

	const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	const FPlayerDefaultInfo* PlayerDefaultInfo = DataSubsystem->GetPlayerDefaultTableInfo();
	if (!PlayerDefaultInfo)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: No valid player default data"), *LOG_LINE_INFO);
		return;
	}

	// TODO: 실제 데이터 처리
}

void APlayerCharacterBase::ApplyRecoil()
{
	// Yaw범위만큼 랜덤
	RecoilPitchCurrent += RecoilPitch;
	RecoilYawCurrent += FMath::RandRange(-RecoilYawRange, RecoilYawRange);
}

void APlayerCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(SkeletalMeshAsset))
	{
		USkeletalMeshComponent* MeshComponent = GetMesh();
		if (IsValid(MeshComponent))
		{
			MeshComponent->SetSkeletalMeshAsset(SkeletalMeshAsset);
		}
		else
		{
			RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: MeshComponent is invalid"), *LOG_LINE_INFO);
		}
	}
	else
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: SkeletalMeshAsset is invalid"), *LOG_LINE_INFO);
	}
}


TSubclassOf<UGameplayEffect> APlayerCharacterBase::GetEffect(const FName TagName) const
{
	if (const TSubclassOf<UGameplayEffect>* FindTag = Effects.Find(TagName))
	{
		return *FindTag;
	}

	return nullptr;
}


void APlayerCharacterBase::SetGenericTeamId(const FGenericTeamId& ID)
{
	TeamID = ID;
}

FGenericTeamId APlayerCharacterBase::GetGenericTeamId() const
{
	return TeamID;
}

ETeamAttitude::Type APlayerCharacterBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);

	if (!OtherTeamAgent)
	{
		return ETeamAttitude::Neutral;
	}
	else if (OtherTeamAgent->GetGenericTeamId().GetId() == TeamNeutral)
	{
		return ETeamAttitude::Neutral;
	}

	return GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId()
		       ? ETeamAttitude::Friendly
		       : ETeamAttitude::Hostile;
}

void APlayerCharacterBase::GiveAbilities()
{
	if (IsValid(AimingAbilityClass))
	{
		FGameplayAbilitySpec AimSpec(AimingAbilityClass, 1);
		AimSpec.InputID = static_cast<int32>(ERSInputID::Aim);
		ASC->GiveAbility(AimSpec);
	}


	if (IsValid(AttackAbilityClass))
	{
		FGameplayAbilitySpec AttackSpec(AttackAbilityClass, 1);
		AttackSpec.InputID = static_cast<int32>(ERSInputID::Fire);
		ASC->GiveAbility(AttackSpec);
	}

	if (IsValid(DashAbilityClass))
	{
		FGameplayAbilitySpec DashSpec(DashAbilityClass, 1);
		DashSpec.InputID = static_cast<int32>(ERSInputID::Dash);
		ASC->GiveAbility(DashSpec);
	}

	if (IsValid(ReloadAbilityClass))
	{
		FGameplayAbilitySpec ReloadSpec(ReloadAbilityClass, 1);
		ReloadSpec.InputID = static_cast<int32>(ERSInputID::Reload);
		ASC->GiveAbility(ReloadSpec);
	}

	if (IsValid(SubSkillAbilityClass))
	{
		FGameplayAbilitySpec SubSkillSpec(SubSkillAbilityClass, 1);
		SubSkillSpec.InputID = static_cast<int32>(ERSInputID::SubSkill);
		ASC->GiveAbility(SubSkillSpec);
	}
	if (IsValid(SubSkillAbilityClass))
	{
		FGameplayAbilitySpec JumpSpec(JumpAbilityClass, 1);
		JumpSpec.InputID = static_cast<int32>(ERSInputID::Jump);
		ASC->GiveAbility(JumpSpec);
	}

	if (IsValid(DeadAbilityClass))
	{
		FGameplayAbilitySpec DeadSpec(DeadAbilityClass, 1);
		ASC->GiveAbility(DeadSpec);
	}
}

void APlayerCharacterBase::SetAimAlpha(float NewAimAlpha)
{
	if (AimAlpha == NewAimAlpha)
	{
		return;
	}

	AimAlpha = NewAimAlpha;

	OnAimAlphaChanged.Broadcast(NewAimAlpha);
}

void APlayerCharacterBase::BindGameplayTagDelegate()
{
	if (!ASC)
	{
		return;
	}

	ASC->RegisterGameplayTagEvent(RSTags::Player::Aiming, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &ThisClass::OnAimingTagChanged);
	ASC->RegisterGameplayTagEvent(RSTags::Player::Attacking, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &ThisClass::OnAttackingTagChanged);
	ASC->RegisterGameplayTagEvent(RSTags::Player::Dashing, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &ThisClass::OnDashingTagChanged);
	ASC->RegisterGameplayTagEvent(RSTags::Player::Reloading, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &ThisClass::OnReloadingTagChanged);
	ASC->RegisterGameplayTagEvent(RSTags::Player::SubSkilling, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &ThisClass::OnSubSkillingTagChanged);
	ASC->RegisterGameplayTagEvent(RSTags::Player::Jumping, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &ThisClass::OnJumpingTagChanged);
}

void APlayerCharacterBase::OnAimingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bool bNewIsAiming = NewCount > 0;

	if (GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
	{
		UpdatePlayerCameraState();
		UpdatePlayerSpeedState();
	}

	if (AimAlphaCurve)
	{
		if (bNewIsAiming)
		{
			AimAlphaTimeline.Play();
		}
		else
		{
			AimAlphaTimeline.Reverse();
		}
	}
}

void APlayerCharacterBase::OnAttackingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UpdatePlayerCameraState();
	UpdatePlayerSpeedState();
}

void APlayerCharacterBase::OnDashingTagChanged(const FGameplayTag Tag, int32 NewCount)
{

	UpdatePlayerCameraState();
	UpdatePlayerSpeedState();
}

void APlayerCharacterBase::OnReloadingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UpdatePlayerCameraState();
	UpdatePlayerSpeedState();
}

void APlayerCharacterBase::OnSubSkillingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UpdatePlayerCameraState();
	UpdatePlayerSpeedState();
}

void APlayerCharacterBase::OnJumpingTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	UpdatePlayerCameraState();
	UpdatePlayerSpeedState();
}

void APlayerCharacterBase::ExecuteMuzzleEffect(const FVector& MuzzleLocation, const FRotator& MuzzleRotation)
{
	if (!WeaponMeshComponent)
		return;
	WeaponMeshComponent->UpdateComponentToWorld();

	// 로컬 전용 GameplayCue 파라미터 구성
	FGameplayCueParameters CueParams;
	CueParams.Location = MuzzleLocation;
	CueParams.Normal = MuzzleRotation.Vector();
	// AI가 들을 수 있는 함수
	MakeNoise(20.f,this, MuzzleLocation,50.f);
	if (ASC)
	{
		ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Weapon.MuzzleFlash")), CueParams);
		ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Weapon.MuzzleSmoke")), CueParams);
	}
}

void APlayerCharacterBase::PlayFootStepSound()
{

	FHitResult Hit;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 200.f);


	//피지컬 머티리얼을 히트 구조체에 담기 위한 설정
	//발 소켓에서 발사한 라인이 내 발바닥 표면에 충돌하는 것을 막는다.
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bReturnPhysicalMaterial = true;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, QueryParams))
	{
		FString PMName = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->GetName() : TEXT("NULL");
		if (Hit.PhysMaterial.IsValid())
		{

			USoundBase* Sound = FootstepSoundMap.FindRef(Hit.PhysMaterial.Get());
			UGameplayStatics::PlaySoundAtLocation(this, Sound, Hit.ImpactPoint);
		}
		else
		{
		}
	}

}

void APlayerCharacterBase::ExecuteBulletTrailEffect(const FVector& MuzzleLocation,const FVector& TrailEnd)
{
	if (ProjectileClass)
	{
		FVector Direction = (TrailEnd - MuzzleLocation).GetSafeNormal();
		FRotator BulletRoation = Direction.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// 3. 월드에 총알 스폰
		GetWorld()->SpawnActor<ABulletProjectile>(ProjectileClass, MuzzleLocation, BulletRoation, SpawnParams);
	}
}

void APlayerCharacterBase::ExecuteDecalEffect(const FHitResult& Hit)
{
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddHitResult(Hit);

	FGameplayCueParameters CueParams;
	CueParams.EffectContext = Context;
	CueParams.Location = Hit.ImpactPoint;
	CueParams.Normal = Hit.ImpactNormal;

	FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Weapon.Impact"));
	ASC->ExecuteGameplayCue(CueTag, CueParams);
}

void APlayerCharacterBase::ExecuteHitEffect(const FHitResult& Hit)
{
	FGameplayEffectContextHandle ParticleEffectContext = ASC->MakeEffectContext();
	ParticleEffectContext.AddHitResult(Hit);

	FGameplayCueParameters CueParams;
	CueParams.EffectContext = ParticleEffectContext;
	CueParams.Location = Hit.ImpactPoint;
	CueParams.Normal = Hit.ImpactNormal;

	FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Damage"));

	ASC->ExecuteGameplayCue(CueTag, CueParams);
}

bool APlayerCharacterBase::CalculateHit(FHitResult& Hit)
{
	float Dist = AttributeSet->GetAttackDistance();

	FVector CameraLocation;
	FRotator CameraRotation;
	if (IsValid(PlayerController))
	{
		// 로컬 클라이언트: 카메라 기준
		// 카메라 위치가 어디있고 회전이 얼마나된지 가져오기.
		PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
	}
	else
	{
		// 서버에서 원격 캐릭터: 캐릭터 눈 위치 + 컨트롤 로테이션으로 대체
		CameraLocation = GetPawnViewLocation();
		CameraRotation = GetBaseAimRotation();
	}

	// 카메라 방향으로 Ray쏘기위해 카메라 위치에서 방향에 거리만큼 곱해주기
	FVector CameraEnd = CameraLocation + (CameraRotation.Vector() * Dist);

	FHitResult CameraHit;
	FCollisionQueryParams Params(NAME_None, false, this);
	Params.bReturnPhysicalMaterial = true;

	// 카메라 시작점부터 카메라가 보는방향의 끝 점 까지 히트된게 있는지 확인
	GetWorld()->LineTraceSingleByChannel(CameraHit, CameraLocation, CameraEnd, ECC_GameTraceChannel1, Params);

	// 카메라가 조준한 물체가 있다면 그 맞은 지점을, 없다면 그냥 사거리 끝을 목표물 위치로 설정
	FVector TargetLocation;
	if (CameraHit.bBlockingHit)
	{
		TargetLocation = CameraHit.ImpactPoint;
	}
	else
	{
		TargetLocation = CameraEnd;
	}
	// 무기 메쉬에서 실제 총알이 나갈 총구소켓의 위치 가져오기
	FVector MuzzleLocation = WeaponMeshComponent->GetSocketLocation(FName("Muzzle"));
	FRotator MuzzleRotation = WeaponMeshComponent->GetSocketRotation(FName("Muzzle"));

	// 카메라가 조준한 위치로 Muzzle에서부터 쏠 방향 구하기(Normalize) -> 방향만 구하기 때문에
	FVector ShootDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();

	// Muzzle에서 카메라가 조준한 방향까지 구한 벡터에서 거리만큼 곱해주기
	FVector FinalEnd = MuzzleLocation + (ShootDirection * Dist);

	// 총구에서 카메라 조준점 끝까지의 히트를 확인하기
	return GetWorld()->LineTraceSingleByChannel(Hit, MuzzleLocation, FinalEnd, ECC_GameTraceChannel1, Params);
}

void APlayerCharacterBase::Multicast_ExecuteVisualEffect_Implementation(const FHitResult& Hit, bool bIsHit)
{
	FGameplayCueParameters CueParams;
	CueParams.Location = Hit.ImpactPoint;
	CueParams.Normal = Hit.ImpactNormal;
	CueParams.PhysicalMaterial = Hit.PhysMaterial;

	// 타격인지 데칼(벽)인지 태그로 구분하여 실행
	FGameplayTag Tag = bIsHit
		? FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Damage"))
		: FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Weapon.Impact"));

	UGameplayCueManager::ExecuteGameplayCue_NonReplicated(this, Tag, CueParams);
}

void APlayerCharacterBase::Server_ShootAttackRay_Implementation()
{
	// 충돌결과를 받을 구조체
	FHitResult Hit;

	if (CalculateHit(Hit))
	{
		AActor* TargetActor = Hit.GetActor();
		ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*TargetActor);

		// 맞은 액터(TargetActor) 가져오기
		if (!TargetActor)
		{
			return;
		}
		// 적군인지 확인
		if (Attitude == ETeamAttitude::Hostile)
		{
			Server_ApplyDamage(TargetActor, Hit,GetAttributeSet()->GetAttackPower());
			Multicast_ExecuteVisualEffect(Hit, true);
		}
		// 아닐경우 벽으로 판정
		else
		{
			Multicast_ExecuteVisualEffect(Hit, false);
		}
	}
	else
	{
	}
}

//디버그용 ShootRay
void APlayerCharacterBase::ShootAttackRay()
{
	if (!IsValid(PlayerController))
		return;

	ApplyRecoil();

	if (!IsLocallyControlled())
		return;

	FHitResult Hit;

	FVector CameraLocation;
	FRotator CameraRotation;


	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	float Dist = AttributeSet->GetAttackDistance();

	FVector CameraEnd = CameraLocation + (CameraRotation.Vector() * Dist);

	FHitResult CameraHit;

	FCollisionQueryParams Params(NAME_None, false, this);
	Params.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(CameraHit, CameraLocation, CameraEnd, ECC_GameTraceChannel1, Params);


	FVector TargetLocation;
	if (CameraHit.bBlockingHit)
	{
		TargetLocation = CameraHit.ImpactPoint;
	}
	else
	{
		TargetLocation = CameraEnd;
	}

	FVector MuzzleLocation = WeaponMeshComponent->GetSocketLocation(FName("Muzzle"));
	FRotator MuzzleRotation = WeaponMeshComponent->GetSocketRotation(FName("Muzzle"));

	FVector ShootDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();

	FVector FinalEnd = MuzzleLocation + (ShootDirection * Dist);

	FVector TrailEnd = FinalEnd;

	if (CalculateHit(Hit))
	{
		TrailEnd = Hit.ImpactPoint;
		Server_ShootAttackRay();
	}
	else
	{
	}
	ExecuteBulletTrailEffect(MuzzleLocation, TrailEnd);
}

void APlayerCharacterBase::Server_ApplyDamage_Implementation(AActor* TargetActor, FHitResult Hit,float Damage)
{
	auto* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddHitResult(Hit);

	TSubclassOf<UGameplayEffect> Effect = GetEffect(TEXT("Damage"));

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Effect, 1.0f, EffectContext);
	SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.HP")),
	                                               -Damage);

	ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

void APlayerCharacterBase::OnPlayerDefaultInfoLoaded()
{
	if (!ASC || !AttributeSet)
	{
		RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: ASC or AttributeSet is nullptr"), *LOG_LINE_INFO);
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>())
		{
			if (const FPlayerDefaultInfo* Info = DataSubsystem->GetPlayerDefaultTableInfo())
			{
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(BaseStatEffectClass, 1.0f, Context);

				if (SpecHandle.IsValid() && SpecHandle.Data.IsValid())
				{
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::AttackPower, Info->AttackPower);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::MaxHP, Info->MaxHP);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::CurrentHP, Info->MaxHP);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::WalkSpeed, Info->WalkSpeed);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::DashSpeed, Info->DashSpeed);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::MaxRange, Info->MaxRange);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::DefensePower, Info->DefensePower);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::MaxAmmo, Info->Ammo);
					SpecHandle.Data->SetSetByCallerMagnitude(RSTags::Common::CurrentAmmo, Info->Ammo);
					RS_LOG_DEBUG(LogRSProperties, Log, TEXT("%s: Ammo: %d"), *LOG_LINE_INFO, Info->Ammo);

					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
				}

				NotifyASCReady();
			}
		}
	}
}

void APlayerCharacterBase::ShowInventory()
{
	ToggleInventory(true);
}

void APlayerCharacterBase::HideInventory()
{
	ToggleInventory(false);
}

void APlayerCharacterBase::ToggleInventory(bool bShow)
{
	auto* PC = GetController<APlayerControllerBase>();
	if (!PC)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get player controller"), *LOG_LINE_INFO);
		return;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get local player"), *LOG_LINE_INFO);
		return;
	}

	auto* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get UI subsystem"), *LOG_LINE_INFO);
		return;
	}

	auto* HUD = Cast<UBattleHUDWidget>(UISubsystem->FindWidget(FName(TEXT("battle_hud"))));
	if (!HUD)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get battle HUD widget"), *LOG_LINE_INFO);
		return;
	}

	UInventoryPopupWidget* InventoryPopupWidget = HUD->GetInventoryPopupWidget();
	if (!InventoryPopupWidget)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get inventory popup widget"), *LOG_LINE_INFO);
		return;
	}

	InventoryPopupWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (bShow)
	{
		InventoryPopupWidget->RefreshInventory();

		PC->SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		FInputModeGameOnly InputMode{};
		InputMode.SetConsumeCaptureMouseDown(true);
		PC->SetInputMode(InputMode);
	}

	RS_LOG_DEBUG(LogRSUI, Log, TEXT("%s: Inventory popup widget visibility set to %s"), *LOG_LINE_INFO, bShow ? TEXT("visible") : TEXT("collapsed"));

	PC->bShowMouseCursor = bShow;
	ToggleCombatWidgets(!bShow);
}

void APlayerCharacterBase::ToggleCombatWidgets(bool bShow)
{
	RS_LOG_DEBUG(LogRSUI, Log, TEXT("%s: Combat widgets visibility set to %s"), *LOG_LINE_INFO, bShow ? TEXT("visible") : TEXT("collapsed"));
	const auto* PC = GetController<APlayerControllerBase>();
	if (!PC)
	{
		RS_LOG_DEBUG(LogRSUI, Error, TEXT("%s: Failed to get player controller"), *LOG_LINE_INFO);
		return;
	}

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		RS_LOG_DEBUG(LogRSUI, Error, TEXT("%s: Failed to get local player"), *LOG_LINE_INFO);
		return;
	}

	const auto* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
	{
		RS_LOG_DEBUG(LogRSUI, Error, TEXT("%s: Failed to get UI subsystem"), *LOG_LINE_INFO);
		return;
	}

	TArray<TObjectPtr<URSUserWidget>> OutCombatWidgets;
	if (UISubsystem->FindCombatWidget(OutCombatWidgets) > 0)
	{
		for (const auto& CombatWidget : OutCombatWidgets)
		{
			CombatWidget->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			RS_LOG_DEBUG(LogRSUI, Log, TEXT("%s: %s visibility set to %s"), *LOG_LINE_INFO, *CombatWidget->GetName(), bShow ? TEXT("Visible") : TEXT("Collapsed"));
		}
	}
}

void APlayerCharacterBase::OnDead()
{
	bIsDead = true;

	RS_PRINT_SCREEN(10.f, FColor::Green, TEXT("OnDead"));

	GetMesh()->SetVisibility(false, true);
	GetMesh()->SetHiddenInGame(true, true);

	HideWeapon();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	if (IsValid(PlayerController))
	{
		PlayerController->Server_RequestSpectateTargets();
	}

}
