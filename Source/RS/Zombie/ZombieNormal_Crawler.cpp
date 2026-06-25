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

	double BoxHeight = 30.0;
	if (auto* Box = Cast<UBoxComponent>(Body))
	{
		BoxHeight = Box->GetUnscaledBoxExtent().Z;
	}

	Mesh->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -BoxHeight), FRotator(0.0, -90.0, 0.0));
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
