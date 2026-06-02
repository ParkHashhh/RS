// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "RS/Input/InputData.h"
#include "Attribute/PlayerAttributeSet.h"
#include "DrawDebugHelpers.h"

// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));

	// SpringArm의 부모 컴포넌트로 Mesh를 지정한다.
	SpringArm->SetupAttachment(RootComponent);

	SpringArm->TargetArmLength = 150.f;
	//mSpringArm->SocketOffset.Y = 20.0;
	SpringArm->SetRelativeLocation(FVector(0.0, 0.0, 200.f));
	SpringArm->SetRelativeRotation(FRotator(-10.0, 90.0, 0.0));

	SpringArm->bUsePawnControlRotation = true;
	
	// CameraComponent를 생성하고 SpringArm에 붙여준다.
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	Camera->SetupAttachment(SpringArm);

	bUseControllerRotationYaw = true;

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);


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
		// 💡 중요: 서버/클라이언트 모두 본인의 액터 정보를 ASC에 등록해야 타이머가 정상 작동합니다.
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
}
void APlayerCharacterBase::MoveKey(const FInputActionValue& Value)
{

	FVector2D MovementVector = Value.Get<FVector2D>();

	AddMovementInput(GetActorForwardVector(), MovementVector.X);

	AddMovementInput(GetActorRightVector(), MovementVector.Y);
}

void APlayerCharacterBase::DashKey(const FInputActionValue& Value)
{
	bool Dash = Value.Get<bool>();

	if (Dash)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("DashKey"));
	}
}

void APlayerCharacterBase::AttackKey(const FInputActionValue& Value)
{
	bool Attack = Value.Get<bool>();

	if (Attack)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("AttackKey"));
	}

	if (ASC)
	{
		FGameplayEventData EventData;
		EventData.Instigator = this; // 이 스킬을 발동한 주체 (나)
		EventData.Target = this;     // 이 스킬이 꽂히는 대상 (우선 나 자신으로 세팅)

		ASC->HandleGameplayEvent(FGameplayTag::RequestGameplayTag(TEXT("Player.Battle.Attack")), &EventData);
	}
}

void APlayerCharacterBase::SitKey(const FInputActionValue& Value)
{
	bool Sit = Value.Get<bool>();

	if (Sit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("SitKey"));
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

// Called every frame
void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	UInputAction* Action = InputData->FindAction(TEXT("Move"));

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::MoveKey);
	}

	Action = InputData->FindAction(TEXT("Dash"));

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::DashKey);
	}

	Action = InputData->FindAction(TEXT("Attack"));

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::AttackKey);
	}

	Action = InputData->FindAction(TEXT("Sit"));

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::SitKey);
	}

	Action = InputData->FindAction(TEXT("Rotation"));

	if (Action)
	{
		EnhancedInput->BindAction(Action, ETriggerEvent::Triggered, this, &APlayerCharacterBase::RotationMouse);
	}

}

void APlayerCharacterBase::Attack_Server_Implementation()
{

	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Attack_Server"));
	Attack_Execution();

}

void APlayerCharacterBase::Attack_Execution_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Attack_Execution"));

}

void APlayerCharacterBase::ShootAttackRay()
{
	// 1. Raycast 구현 (LineTrace)
	FHitResult Hit;
	FVector Start = GetActorLocation(); // 포켓이 들어가야함

	float Dist = AttributeSet->GetAttackDistance();
	FVector End = Start + (GetActorForwardVector() * Dist);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_GameTraceChannel1);

	// 🟢 디버그 라인 그리기
	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 2.0f, 0, 2.0f);

	// 🟢 결과 판단도 bHit 변수로 수행
	if (bHit)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("🎯 적중!"));
	}
}
