// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AbilitySystemComponent.h"


// Sets default values
AZombieBase::AZombieBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//필요한 부품들을 생성한다.
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	SetRootComponent(Body);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Body);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Body);

	//ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	//AttributeSet = CreateDefaultSubobject<UZombieAttributeSet>(TEXT("AttributeSet"));
	//ASC->AddAttributeSetSubobject<UZombieAttributeSet>(AttributeSet);

	SetGenericTeamId(FGenericTeamId(TeamZombie));
}

// Called when the game starts or when spawned
void AZombieBase::BeginPlay()
{
	Super::BeginPlay();

	//AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());

	//ASC->InitAbilityActorInfo(this, this);
	//ASC->GiveAbility(FGameplayAbilitySpec(UGameplayAbility_NormalAttack::StaticClass(), 1, 0));
	
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

