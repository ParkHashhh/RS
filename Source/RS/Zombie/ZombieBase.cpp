// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AbilitySystemComponent.h"
#include "AI/ZombieAIController.h"


// Sets default values
AZombieBase::AZombieBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//필요한 부품들을 생성한다.
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	Body->SetCollisionProfileName(ZombieCollisionName);
	Body->AddRelativeLocation(FVector(0, 0, 50));

	SetRootComponent(Body);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Body);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Body);

	bUseControllerRotationYaw = true;

	//객체별 AttributeSet의 타입이 다르므로 AttributeSet의 생성, 연결은 자식 객체에서 진행해야 한다.
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	ASC->SetIsReplicated(true);
	SetReplicates(true);
	SetReplicateMovement(true);

	SetGenericTeamId(FGenericTeamId(TeamMonster));
		
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AZombieBase::BeginPlay()
{
	Super::BeginPlay();

	//AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());

	//ASC->InitAbilityActorInfo(this, this);
	//ASC->GiveAbility(FGameplayAbilitySpec(UGameplayAbility_NormalAttack::StaticClass(), 1, 0));
	
}

void AZombieBase::PostInitProperties()
{
	Super::PostInitProperties();

	//디테일에서 지정한 값은 이 타이밍에 들어가게 된다.
	//이곳에서 지정한 ai컨트롤러를 사용하도록 설정해야 한다.
	if (IsValid(AICtrl))
	{
		AIControllerClass = AICtrl;
	}
}

// Called every frame
void AZombieBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AZombieBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AZombieBase::PossessedBy(AController* NewController)
{
	//컨트롤러 빙의가 성공했을 때 비헤이비어 트리를 바로 활성화 시켜줘야 한다.
	AZombieAIController* Ctrl = Cast<AZombieAIController>(NewController);
	GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, TEXT("Test"));
	if (IsValid(Ctrl))
	{
		Ctrl->ActivateBT();
	}
	Super::PossessedBy(NewController);
}

void AZombieBase::SetGenericTeamId(const FGenericTeamId& ID)
{
	TeamID = ID;
}

FGenericTeamId AZombieBase::GetGenericTeamId() const
{
	return TeamID;
}

ETeamAttitude::Type AZombieBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);

	if (!OtherTeamAgent)
		return ETeamAttitude::Neutral;

	else if (OtherTeamAgent->GetGenericTeamId().GetId() == TeamNeutral)
		return ETeamAttitude::Neutral;

	return GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId() ?
		ETeamAttitude::Friendly : ETeamAttitude::Hostile;
}

