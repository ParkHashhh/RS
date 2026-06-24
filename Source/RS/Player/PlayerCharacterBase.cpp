// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"

#include "Attribute/PlayerAttributeSet.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Input/InputData.h"
#include "RS/Zombie/ZombieBase.h"
#include "GameplayCueManager.h"

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
	WeaponMeshComponent->SetIsReplicated(true);
	if (AimAlphaCurve)
	{
		FOnTimelineFloat OnTimelineFloat;
		OnTimelineFloat.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(APlayerCharacterBase, SetAimAlpha));
		AimAlphaTimeline.AddInterpFloat(AimAlphaCurve, OnTimelineFloat);
	}

	if (IsASCReady())
	{
		BindGameplayTagDelegate();
	}
	else
	{
		OnASCReady.AddUObject(this, &ThisClass::BindGameplayTagDelegate);
	}


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
		BindGameplayTagDelegate();
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
	SpringArm->SetRelativeLocation(FMath::VInterpTo(SpringArm->GetRelativeLocation(), TargetCameraPos, DeltaTime,
	                                                InterpSpeed));
	Camera->SetRelativeRotation(
		FMath::RInterpTo(Camera->GetRelativeRotation(), TargetCameraRot, DeltaTime, InterpSpeed));

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

void APlayerCharacterBase::OnIsDashingChanged() const
{
	OnDashingChanged.Broadcast(bIsDashing);
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
	if (!IsValid(ASC) || !GetMovementComponent() || GetMovementComponent()->IsFalling())
	{
		return;
	}
	ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Fire));
}


void APlayerCharacterBase::AttackReleaseKey(const FInputActionValue& Value)
{
	ASC->AbilityLocalInputReleased(static_cast<int32>(ERSInputID::Fire));
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
	if (!IsValid(ASC) || !GetMovementComponent() || GetMovementComponent()->IsFalling())
	{
		return;
	}

	ASC->AbilityLocalInputPressed(static_cast<int32>(ERSInputID::Aim));

}

void APlayerCharacterBase::AimReleaseKey(const FInputActionValue& Value)
{
	if (!IsValid(ASC))
	{
		return;
	}

	ASC->AbilityLocalInputReleased(static_cast<int32>(ERSInputID::Aim));

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
		WeaponMeshComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale,
		                                       SocketName);
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

	ASC->RegisterGameplayTagEvent(RSTags::Player::Aiming, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnAimingTagChanged);
	ASC->RegisterGameplayTagEvent(RSTags::Player::Attacking, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnAttackingTagChanged);
	ASC->RegisterGameplayTagEvent(RSTags::Player::Dashing, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnDashingTagChanged);
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
	if (GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
	{
		UpdatePlayerCameraState();
		UpdatePlayerSpeedState();
	}
}

void APlayerCharacterBase::OnDashingTagChanged(const FGameplayTag Tag, int32 NewCount)
{


	if (GetCharacterMovement() && !GetCharacterMovement()->IsFalling())
	{
		UpdatePlayerCameraState();
		UpdatePlayerSpeedState();
	}
}

void APlayerCharacterBase::ExecuteMuzzleEffect()
{
	if (!WeaponMeshComponent)
		return;
	WeaponMeshComponent->UpdateComponentToWorld();

	FVector MuzzleLocation = WeaponMeshComponent->GetSocketLocation(FName("Muzzle"));
	FRotator MuzzleRotation = WeaponMeshComponent->GetSocketRotation(FName("Muzzle"));

	// 로컬 전용 GameplayCue 파라미터 구성
	FGameplayCueParameters CueParams;
	CueParams.Location = MuzzleLocation;
	CueParams.Normal = MuzzleRotation.Vector();

	if (ASC)
	{
		ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Weapon.MuzzleFlash")), CueParams);
		ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Weapon.MuzzleSmoke")), CueParams);
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
	// 캐릭터 컨트롤러 가져오기
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
		// 사거리 가져오기
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
			Server_ApplyDamage(TargetActor, Hit);
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
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (!IsValid(PlayerController))
		return;

	ApplyRecoil();
	ExecuteMuzzleEffect();

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

	if (CalculateHit(Hit))
	{
		Server_ShootAttackRay();
		DrawDebugLine(GetWorld(), MuzzleLocation, Hit.Location, FColor::Red, false, 0.2f, 0, 2.0f);
	}
	else
	{
		DrawDebugLine(GetWorld(), MuzzleLocation, FinalEnd, FColor::Green, false, 0.2f, 0, 2.0f);
	}
}

void APlayerCharacterBase::Server_ApplyDamage_Implementation(AActor* TargetActor, FHitResult Hit)
{
	auto* TargetChar = Cast<AZombieBase>(TargetActor);

	if (!TargetChar)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();
	if (!TargetASC)
	{
		return;
	}

	// 상대방 액터를 좀비로 형변환
	float Damage = AttributeSet->GetAttack();

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddHitResult(Hit);

	TSubclassOf<UGameplayEffect> Effect = GetEffect(TEXT("Damage"));


	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Effect, 1.0f, EffectContext);
	SpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(TEXT("SetByCaller.State.HP")),
	                                               -Damage);

	ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

}
