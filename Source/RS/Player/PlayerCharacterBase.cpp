// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"

#include "RSTags.h"
#include "Attribute/PlayerAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Input/InputData.h"
#include "Engine/SkeletalMeshSocket.h"
#include "RS/Zombie/ZombieBase.h"

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

	SpringArm->bUsePawnControlRotation = true;

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

	if (!DataSubsystem->IsPlayerDefaultTableLoaded())
	{
		DataSubsystem->OnPlayerDefaultInfoLoaded.AddDynamic(this, &APlayerCharacterBase::LoadDefaultData);
	}
	else
	{
		LoadDefaultData();
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetController());

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
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsValid(ASC))
	{
		ASC->InitAbilityActorInfo(this, this);

		if (DefaultAttributeEffect)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DefaultAttributeEffect, 1.0f, Context);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			}
		}

		if (HasAuthority())
		{
			GiveAbilities();
		}

		NotifyASCReady();
	}
}

void APlayerCharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (IsValid(ASC))
	{
		ASC->InitAbilityActorInfo(this, this);

		NotifyASCReady();
	}
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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

	AnimInst->ViewPitch = FMath::Clamp(RelativePitch, -40.0f, 40.0f);


	float InterpSpeed = 10.f;
	// 현재의 위치에서 타겟위치까지 보간
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaTime, InterpSpeed);
	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, InterpSpeed);
	SpringArm->SetRelativeLocation(FMath::VInterpTo(SpringArm->GetRelativeLocation(), TargetCameraPos, DeltaTime, InterpSpeed));
	Camera->SetRelativeRotation(FMath::RInterpTo(Camera->GetRelativeRotation(), TargetCameraRot, DeltaTime, InterpSpeed));
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

void APlayerCharacterBase::SetIsAiming(bool bNewIsAiming)
{
	if (bIsAiming == bNewIsAiming)
	{
		return;
	}

	bIsAiming = bNewIsAiming;

	if (GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
	{
		UpdatePlayerCameraState();
		UpdatePlayerSpeedState();
	}

	OnIsAimingChanged();
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


void APlayerCharacterBase::PlayBaseAttackMontage()
{
	if (!AnimInst)
	{
		return;
	}

	AnimInst->PlayBaseAttackMontage();
}

void APlayerCharacterBase::OnIsAttackingChanged() const
{
	OnAttackingChanged.Broadcast(bIsAttacking);
}

void APlayerCharacterBase::OnIsDashingChanged() const
{
	OnDashingChanged.Broadcast(bIsDashing);
}

void APlayerCharacterBase::OnIsAimingChanged() const
{
	OnAimingChanged.Broadcast(bIsAiming);
}

void APlayerCharacterBase::MoveKey(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), MovementVector.X);

	AddMovementInput(GetActorRightVector(), MovementVector.Y);

	bIsMoveKey = true;

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
	bIsMoveKey = false;
}

void APlayerCharacterBase::DashKey(const FInputActionValue& Value)
{
	if (CanDash())
	{
		SetIsDashing(true);
	}
}

void APlayerCharacterBase::DashReleaseKey(const FInputActionValue& Value)
{
	SetIsDashing(false);
}

void APlayerCharacterBase::AttackKey(const FInputActionValue& Value)
{
	// Bool 변수로 누르고 있는지 땠는지를 판단 한 후 공격속도마다 공격 함수를 호출하게 하고(이펙트 파티클 등등), 서버에서는 충돌처리 및 데미지 체크하기 => 완료
	if (!GetMovementComponent() || GetMovementComponent()->IsFalling())
	{
		return;
	}

	SetIsAttacking(true);

	// Bool 변수로 누르고 있는지 땠는지를 판단 한 후 공격속도마다 공격 함수를 호출하게 하고(이펙트 파티클 등등), 서버에서는 충돌처리 및 데미지 체크하기 => 완료
	if (IsLocallyControlled() && IsAttacking())
	{
		if (IsValid(ASC))
		{
			FGameplayEventData EventData;
			EventData.Instigator = this; // 이 스킬을 발동한 주체
			EventData.Target = this; // 이 스킬이 꽂히는 대상

			if (!IsAiming())
			{
				SetIsDashing(false);
			}

			ASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Player.Battle.Attack")), &EventData);
		}
	}
}


void APlayerCharacterBase::AttackReleaseKey(const FInputActionValue& Value)
{
	SetIsAttacking(false);
}


void APlayerCharacterBase::JumpKey(const FInputActionValue& Value)
{
	bool bJump = Value.Get<bool>();
	if (bJump)
	{
		if (CanJump())
		{
			Jump();
			AimReleaseKey(Value);
			DashReleaseKey(Value);
		}
		else
		{
		}
	}
}

void APlayerCharacterBase::AimKey(const FInputActionValue& Value)
{
	if (!GetCharacterMovement() || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	if (IsValid(ASC))
	{
		FGameplayTag AimTag = FGameplayTag::RequestGameplayTag(RSTags::Player::AimTagName);
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AimTag));
	}

	SetIsDashing(false);
}

void APlayerCharacterBase::AimReleaseKey(const FInputActionValue& Value)
{
	if (IsValid(ASC))
	{
		FGameplayTag AimTag = FGameplayTag::RequestGameplayTag(RSTags::Player::AimTagName);
		FGameplayTagContainer TargetContainer(AimTag);

		ASC->CancelAbilities(&TargetContainer);
	}
}

void APlayerCharacterBase::RotationMouse(const FInputActionValue& Value)
{
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
}

void APlayerCharacterBase::DebugKey1(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey1"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey1"));
}

void APlayerCharacterBase::DebugKey2(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey2"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey2"));
}

void APlayerCharacterBase::DebugKey3(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey3"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey3"));
}

void APlayerCharacterBase::DebugKey4(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey4"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey4"));
}

void APlayerCharacterBase::DebugKey5(const FInputActionValue& Value)
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("DebugKey5"));
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("DebugKey5"));
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
		WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
	}
}
void APlayerCharacterBase::UpdatePlayerCameraState()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();

	// 1. 공중
	if (MovementComponent->IsFalling())
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
	if (MovementComponent->IsFalling())
	{
		MovementComponent->MaxWalkSpeed = 400.f;
		Server_SetMaxWalkSpeed(400.f);
		return;
	}

	float TargetSpeed = 400.f;

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
void APlayerCharacterBase::ShootAttackRay()
{
	FHitResult Hit;

	// 캐릭터 컨트롤러 가져오기
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!IsValid(PlayerController))
		return; 

	// 사거리 가져오기
	float Dist = AttributeSet->GetAttackDistance();

	FVector CameraLocation;
	FRotator CameraRotation;

	// 카메라 위치가 어디있고 회전이 얼마나된지 가져오기.
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// 카메라 방향으로 Ray쏘기위해 카메라 위치에서 방향에 거리만큼 곱해주기
	FVector CameraEnd = CameraLocation + (CameraRotation.Vector() * Dist);

	FHitResult CameraHit;
	FCollisionQueryParams Params(NAME_None, false, this);

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

	// 카메라가 조준한 위치로 Muzzle에서부터 쏠 방향 구하기(Normalize) -> 방향만 구하기 때문에
	FVector ShootDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();

	// Muzzle에서 카메라가 조준한 방향까지 구한 벡터에서 거리만큼 곱해주기
	FVector FinalEnd = MuzzleLocation + (ShootDirection * Dist);

	// 총구에서 카메라 조준점 끝까지의 히트를 확인하기 
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, MuzzleLocation, FinalEnd, ECC_GameTraceChannel1, Params);
	if (bHit)
	{
		if (IsLocallyControlled())
		{
			DrawDebugLine(GetWorld(), MuzzleLocation, Hit.Location, FColor::Red, false, 0.2f, 0, 2.0f);
		}

		// 맞은 액터(TargetActor) 가져오기
		AActor* TargetActor = Hit.GetActor();
		if (!TargetActor)
		{
			return;
		}

		// 적군인지 확인
		ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*TargetActor);

		if (Attitude == ETeamAttitude::Hostile)
		{
			// 적일경우 데미지를 주기 위해 ASC를 가져온다.
			UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();

			if (TargetASC && ASC)
			{
				Server_ApplyDamage(TargetActor, Hit);
			}
		}
	}
	else
	{
		// 맞지 않았을때
		if (IsLocallyControlled())
		{
			DrawDebugLine(GetWorld(), MuzzleLocation, FinalEnd, FColor::Green, false, 0.2f, 0, 2.0f);
		}
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
	for (TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		if (IsValid(AbilityClass))
		{
			ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, -1));
		}
	}

	if (IsValid(AimingAbilityClass))
	{
		ASC->GiveAbility(FGameplayAbilitySpec(AimingAbilityClass, 1, -1));
	}
}

void APlayerCharacterBase::Server_ApplyDamage_Implementation(AActor* TargetActor, FHitResult Hit)
{
	AZombieBase* TargetChar = Cast<AZombieBase>(TargetActor);

	UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();

	if (HasAuthority())
	{
		// Attribute 데미지 감소 처리 => 서버에서만 해야함

		// 상대방 액터를 좀비로 형변환
		float Damage = AttributeSet->GetAttack();

		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddHitResult(Hit);

		TSubclassOf<UGameplayEffect> Effect = GetEffect(TEXT("Damage"));

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Effect, 1.0f, EffectContext);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.HP")), -Damage);

		ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		float CurrentHP = TargetChar->GetAttributeSet()->GetHP();
	}

	FGameplayCueParameters CueParams;
	CueParams.Location = Hit.Location;
	CueParams.Normal = Hit.ImpactNormal;

	FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Damage"));

	TargetASC->ExecuteGameplayCue(CueTag, CueParams);
}
