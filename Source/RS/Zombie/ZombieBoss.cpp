// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBoss.h"
#include "GAS/ZombieBossAttributeSet.h"


// Sets default values
AZombieBoss::AZombieBoss()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttributeSet = CreateDefaultSubobject<UZombieBossAttributeSet>(TEXT("Attribute"));
	UZombieBossAttributeSet* BossAttri = Cast<UZombieBossAttributeSet>(AttributeSet);
	if (IsValid(BossAttri))
	{
		ASC->AddAttributeSetSubobject<UZombieBossAttributeSet>(BossAttri);
	}
}

// Called when the game starts or when spawned
void AZombieBoss::BeginPlay()
{
	Super::BeginPlay();

	BossAttributeSet = Cast<UZombieBossAttributeSet>(AttributeSet);
	
}

// Called every frame
void AZombieBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AZombieBoss::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AZombieBoss::Attack()
{

}
