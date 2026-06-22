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
	Movement->SetUpdatedComponent(Body);

	Cast<UCapsuleComponent>(Body)->SetCapsuleHalfHeight(75.f);
	Cast<UCapsuleComponent>(Body)->SetCapsuleRadius(30.f);

	//캡슐은 무브먼트와 네비 메쉬의 보정 수치가 10정도 된다.
	Mesh->SetRelativeLocation(FVector(0.0, 0.0, -85.f));
	Mesh->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));

	if (IsValid(NormalAttributeSet))
	{
		NormalAttributeSet->InitWalkSpeed(150.f);
		NormalAttributeSet->InitRunSpeed(200.f);
	}
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
