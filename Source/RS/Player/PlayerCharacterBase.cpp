// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "RS/Input/InputData.h"
#include "Attribute/PlayerAttributeSet.h"
#include "DrawDebugHelpers.h"
#include "RS/LogCategory.h"
#include "RS/Input/InputData.h"
#include "RS/Zombie/ZombieBase.h"

// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Arm"));
	// SpringArm의 부모 컴포넌트로 Capsule을 지정한다.
	SpringArm->SetupAttachment(GetMesh());

	SpringArm->TargetArmLength = 150.f;
	SpringArm->SetRelativeLocation(FVector(0.0, 0.0, 150.f));
	SpringArm->SetRelativeRotation(FRotator(-10.0, 90.0, 0.0));

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
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacterBase, IsAttacking); // 🎯 동기화 대상 등록
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

	// TODO : Bool 변수로 누르고 있는지 땠는지를 판단 한 후 공격속도마다 공격 함수를 호출하게 하고(이펙트 파티클 등등), 서버에서는 충돌처리 및 데미지 체크하기
	IsAttacking = true;

	SetIsAttacking(true); // Server RPC 함수 필요

	if (IsLocallyControlled() && IsAttacking)
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
	IsAttacking = false;
	SetIsAttacking(false);
	RS_PRINT_SCREEN(2, FColor::Red, TEXT("Release Attack Key"));
}

void APlayerCharacterBase::SetIsAttacking_Implementation(bool Attacking)
{
	IsAttacking = Attacking;
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
		RS_PRINT_SCREEN(1.0f, FColor::Cyan, TEXT("💥 좀비 잔여 체력: %f"), CurrentHP);
	}
	FGameplayCueParameters CueParams;
	CueParams.Location = Hit.Location;
	CueParams.Normal = Hit.ImpactNormal;

	// 호출하고 싶은 게임플레이 큐 태그를 정확하게 적어줍니다.
	FGameplayTag CueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Player.Damage")); // 주석님의 큐 태그 이름으로!

	// ⚡ 상대방(좀비)의 ASC에게 직접 이 태그의 큐 연출을 터트리라고 명령합니다!
	TargetASC->ExecuteGameplayCue(CueTag, CueParams);
}

