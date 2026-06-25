// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal_Stand.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GAS/ZombieStandardAttributeSet.h"

AZombieNormal_Stand::AZombieNormal_Stand()
{
	//스탠딩 좀비는 캡슐 컴포넌트를 사용한다.
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	Body->SetCollisionProfileName(ZombieCollisionName);
	Mesh->SetupAttachment(Body);

	double HalfHeight = 75.0;
	if (auto* Capsule = Cast<UCapsuleComponent>(Body))
	{
		HalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	}

	Mesh->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -HalfHeight), FRotator(0.0, -90.0, 0.0));
}

void AZombieNormal_Stand::BeginPlay()
{
	Super::BeginPlay();

	ChangeSpeed_Walk();
}

void AZombieNormal_Stand::Attack()
{
	Super::Attack();
}
