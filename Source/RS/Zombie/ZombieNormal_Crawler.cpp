// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal_Crawler.h"
#include "Components/BoxComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GAS/ZombieStandardAttributeSet.h"

AZombieNormal_Crawler::AZombieNormal_Crawler()
{
	//기는 좀비는 박스 컴포넌트를 사용한다.
	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Body->SetCollisionProfileName(ZombieCollisionName);
	Mesh->SetupAttachment(Body);
	Movement->SetUpdatedComponent(Body);

	Cast<UBoxComponent>(Body)->SetBoxExtent(FVector(70.0, 30.0, 30.0));

	//박스는 네비메쉬와의 보정치가 캡슐만큼 크지 않다.
	Mesh->SetRelativeLocation(FVector(0.0, 0.0, -35.f));
	Mesh->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));

	if (IsValid(NormalAttributeSet))
	{
		NormalAttributeSet->InitWalkSpeed(50.f);
		NormalAttributeSet->InitRunSpeed(75.f);
	}
}

void AZombieNormal_Crawler::BeginPlay()
{
	Super::BeginPlay();

	ChangeSpeed_Walk();
}

void AZombieNormal_Crawler::Attack()
{
	Super::Attack();
}

