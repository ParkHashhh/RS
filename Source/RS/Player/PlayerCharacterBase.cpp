// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"
#include "RS/Player/Character/PlayerAnimInstance.h"
#include "RS/Input/InputData.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "RS/Input/InputData.h"
#include "Components/CapsuleComponent.h"
#include "Attribute/PlayerAttributeSet.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/LogCategory.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	// SpringArm의 부모 컴포넌트로 Capsule을 지정한다.
	SpringArm->SetupAttachment(GetCapsuleComponent());

	GetMesh()->AddRelativeLocation(FVector(0,0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

	SpringArm->TargetArmLength = 250.f;
	SpringArm->SetRelativeLocation(FVector(0.0, 0.0, 60.f));
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
			PlayerController->PlayerCameraManager->ViewPitchMin = -60.f;
			PlayerController->PlayerCameraManager->ViewPitchMax = 60.f;
		}
	}
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this, this);

		if (DefaultAttributeEffect)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DefaultAttributeEffect, 1.0f, Context);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
		for (TSubclassOf<UGameplayAbility> AbilityClass : Abilitys)
		{
			if (AbilityClass)
			{
				ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, -1));
			}
		}
	}

	SetMouseSensitivity(0.85f, 0.85f);

	CurrentArmLength = SpringArm->TargetArmLength;
	TargetArmLength = SpringArm->TargetArmLength;


	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	Server_SetMaxWalkSpeed(400.f);

	AnimInst = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacterBase, bIsAttacking); // 🎯 동기화 대상 등록
}

// Called every frame
void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float InterpSpeed = 10.0f;

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetArmLength, DeltaTime, InterpSpeed);
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

	Action = InputData->FindAction(FInputKeyNames::Sit);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::SitKey);
	}

	Action = InputData->FindAction(FInputKeyNames::Rotation);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::RotationMouse);
	}

	Action = InputData->FindAction(FInputKeyNames::Aim);

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Started, this, &APlayerCharacterBase::AimKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Completed, this, &APlayerCharacterBase::AimReleaseKey);
		EnhancedInput->BindAction(Action, ETriggerEvent::Canceled, this, &APlayerCharacterBase::AimReleaseKey);
	}

}

void APlayerCharacterBase::Server_SetMaxWalkSpeed_Implementation(float NewSpeed)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
	}
}

void APlayerCharacterBase::MoveKey(const FInputActionValue& Value)
{

	FVector2D MovementVector = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), MovementVector.X);

	AddMovementInput(GetActorRightVector(), MovementVector.Y);

	if (bIsDashing && AnimInst && !AnimInst->GetDirectionFoward())
	{
		bIsDashing = false;

		float TargetSpeed = bIsAiming ? 200.f : 400.f;
		GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
		Server_SetMaxWalkSpeed(TargetSpeed);
	}
}

void APlayerCharacterBase::DashKey(const FInputActionValue& Value)
{

	if (!bIsDashing && !bIsAiming && AnimInst && AnimInst->GetDirectionFoward())
	{
		bIsDashing = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
		Server_SetMaxWalkSpeed(600.f);
	}
		
}

void APlayerCharacterBase::DashReleaseKey(const FInputActionValue& Value)
{
	bIsDashing = false;

	float TargetSpeed = bIsAiming ? 200.f : 400.f;

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("DashReleaseKey"));

	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
	Server_SetMaxWalkSpeed(TargetSpeed);
}

void APlayerCharacterBase::AttackKey(const FInputActionValue& Value)
{

	// Bool 변수로 누르고 있는지 땠는지를 판단 한 후 공격속도마다 공격 함수를 호출하게 하고(이펙트 파티클 등등), 서버에서는 충돌처리 및 데미지 체크하기
	bIsAttacking = true;

	SetIsAttacking(true); // Server RPC 함수 필요

	if (IsLocallyControlled() && bIsAttacking)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("AttackKey"));
		if (ASC)
		{
			FGameplayEventData EventData;
			EventData.Instigator = this; // 이 스킬을 발동한 주체
			EventData.Target = this;     // 이 스킬이 꽂히는 대상

			ASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Player.Battle.Attack")), &EventData);
		}
	}
}

void APlayerCharacterBase::AttackReleaseKey(const FInputActionValue& Value)
{
	bIsAttacking = false;
	SetIsAttacking(false);
	RS_PRINT_SCREEN(2, FColor::Red, TEXT("Release Attack Key"));
}

void APlayerCharacterBase::SetIsAttacking_Implementation(bool Attacking)
{
	bIsAttacking = Attacking;
}


void APlayerCharacterBase::SitKey(const FInputActionValue& Value)
{
	bool Sit = Value.Get<bool>();

	if (Sit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("SitKey"));
	}
}

void APlayerCharacterBase::AimKey(const FInputActionValue& Value)
{
	bIsAiming = true;

	TargetArmLength = 200.f;

	if (GetCharacterMovement())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("AimKey"));

		// 🔴 로컬 속도 변경 + 🔵 서버 속도 변경 요청
		GetCharacterMovement()->MaxWalkSpeed = 200.f;
		Server_SetMaxWalkSpeed(200.f);
		Camera->FieldOfView = 60.f;
		SpringArm->SetRelativeLocation(FVector(0.0, 50.0, 60.f));
	}
}

void APlayerCharacterBase::AimReleaseKey(const FInputActionValue& Value)
{
	bIsAiming = false;

	TargetArmLength = 250.f;

	if (GetCharacterMovement())
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("AimReleaseKey"));
		float TargetSpeed = bIsDashing ? 600.f : 400.f;

		// 🔴 로컬 속도 변경 + 🔵 서버 속도 변경 요청
		GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
		Server_SetMaxWalkSpeed(TargetSpeed);
		Camera->FieldOfView = 90.f;
		SpringArm->SetRelativeLocation(FVector(0.0, 0.0, 60.f));
	}
}

void APlayerCharacterBase::RotationMouse(const FInputActionValue& Value)
{
	FVector	Axis = Value.Get<FVector>();
	AddControllerYawInput(Axis.X * MouseXSensitivity);
	AddControllerPitchInput(Axis.Y * MouseYSensitivity);
}

void APlayerCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (SkeletalMeshAsset)
	{
		GetMesh()->SetSkeletalMeshAsset(SkeletalMeshAsset);
	}
}


// 공격함수
void APlayerCharacterBase::ShootAttackRay()
{
	FHitResult Hit;
	FVector Start = GetActorLocation(); // 소켓이 들어가야함
	float Dist = AttributeSet->GetAttackDistance();
	float HP = AttributeSet->GetHP();

	FVector End = Start + (GetActorForwardVector() * Dist);

	FCollisionQueryParams Params(NAME_None, false, this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1, Params);

	if (bHit)
	{
		if (IsLocallyControlled())
		{
			DrawDebugLine(GetWorld(), Start, Hit.Location, FColor::Red, false, 2.0f, 0, 2.0f);
		}
		AActor* TargetActor = Hit.GetActor();
		if (!TargetActor)
			return;

		ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*TargetActor);

		RS_PRINT_SCREEN(0.2, FColor::Red, TEXT("Hit Actor Name : %s"),*TargetActor->GetName());

		if (Attitude == ETeamAttitude::Hostile)
		{
			UAbilitySystemComponent* TargetASC = TargetActor->FindComponentByClass<UAbilitySystemComponent>();

			if (TargetASC && ASC)
			{
				Apply_Damage_Server(TargetActor,Hit);
			}
		}
	}
	else
	{

		if (IsLocallyControlled())
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f, 0, 2.0f);
		}
		RS_PRINT_SCREEN(0.2, FColor::Black, TEXT("No Hit"));
	}
}

TSubclassOf<UGameplayEffect> APlayerCharacterBase::GetEffect(const FName TagName) const
{
	const TSubclassOf<UGameplayEffect>* FindTag = Effects.Find(TagName);
	if(FindTag)
		return *FindTag;
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
		return ETeamAttitude::Neutral;

	else if (OtherTeamAgent->GetGenericTeamId().GetId() == TeamNeutral)
		return ETeamAttitude::Neutral;

	return GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId() ?
		ETeamAttitude::Friendly : ETeamAttitude::Hostile;
}

void APlayerCharacterBase::Apply_Damage_Server_Implementation(AActor* TargetActor, FHitResult Hit)
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
		RS_PRINT_SCREEN(1.0f, FColor::Cyan, TEXT("Current HP : %f"), CurrentHP);
	}
	FGameplayCueParameters CueParams;
	CueParams.Location = Hit.Location;
	CueParams.Normal = Hit.ImpactNormal;

	FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Damage"));

	TargetASC->ExecuteGameplayCue(CueTag, CueParams);
}

