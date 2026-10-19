// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBoss_Behemoth_PickupPoint.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "RS/Zombie/ZombieBase.h"
#include "NavigationSystem.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AZombieBoss_Behemoth_PickupPoint::AZombieBoss_Behemoth_PickupPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	PickupPoint = CreateDefaultSubobject<USplineComponent>(TEXT("PickupPoint"));

	SetRootComponent(Root);

	PickupPoint->SetupAttachment(Root);

#if WITH_EDITORONLY_DATA

	// 에디터에서 어디 있는지 표시해준다.
	Root->bVisualizeComponent = true;

	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->bTreatAsASprite = true;
		ArrowComponent->SpriteInfo.Category = TEXT("Spawn");
		ArrowComponent->SpriteInfo.DisplayName =
			NSLOCTEXT("SpriteCategory", "Spawn", "Spawn");
		ArrowComponent->SetupAttachment(Root);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}

#endif

}

// Called when the game starts or when spawned
void AZombieBoss_Behemoth_PickupPoint::BeginPlay()
{
	Super::BeginPlay();

	//서버에서만 실행해도 된다.
	if (!HasAuthority())
	{
		Destroy();
		return;
	}
}

// Called every frame
void AZombieBoss_Behemoth_PickupPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZombieBoss_Behemoth_PickupPoint::OnConstruction(const FTransform& Transform)
{
	// 배열을 비워준다.
	PickupPointArray.Empty();

	// 스플라인의 좌표들을 배열로 옮겨 담는다.
	int32	Count = PickupPoint->GetNumberOfSplinePoints();

	for (int32 i = 0; i < Count; ++i)
	{
		FVector	Point = PickupPoint->GetLocationAtSplinePoint(i,
			ESplineCoordinateSpace::World);

		PickupPointArray.Add(Point);
	}
}

void AZombieBoss_Behemoth_PickupPoint::CopyPickupPointArray(TArray<FVector>& Array)
{
	//인자로 받은 배열에 내가 가지고 있는 좌표 배열을 복사해 넘겨준다.
	Array.Empty();
	Array.Append(PickupPointArray);
}

