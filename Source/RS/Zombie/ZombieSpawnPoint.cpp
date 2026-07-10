// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawnPoint.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "ZombieBase.h"
#include "NavigationSystem.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AZombieSpawnPoint::AZombieSpawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	FirstDstList = CreateDefaultSubobject<USplineComponent>(TEXT("FirstDstList"));

	SetRootComponent(Root);

	FirstDstList->SetupAttachment(Root);

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
void AZombieSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		Destroy();
		return;
	}

	//타이머를 이용해 무한 스폰하는 스폰 포인트인 경우 타이머를 설정한다.
	if (bInfiniteSpawnUseTimer && SpawnInterval > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(SpawnTimer, FTimerDelegate::CreateUObject(this, &AZombieSpawnPoint::SpawnZombie), SpawnInterval, true);
	}

	//디폴트 스폰 설정이 되어 있는 경우에만 처음에 스폰을 한다.
	if (bDefaultSpawn)
	{
		FirstSpawnZombie();
	}
}

// Called every frame
void AZombieSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZombieSpawnPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 배열을 비워준다.
	//mPatrolPoints.RemoveAll([](FVector Point) {return true; });
	FirstDstPointArray.Empty();

	// Splie의 점 수를 얻어온다.
	int32	Count = FirstDstList->GetNumberOfSplinePoints();

	for (int32 i = 0; i < Count; ++i)
	{
		FVector	Point = FirstDstList->GetLocationAtSplinePoint(i,
			ESplineCoordinateSpace::World);

		FirstDstPointArray.Add(Point);
	}
}

void AZombieSpawnPoint::SpawnTimerFinish()
{

}

void AZombieSpawnPoint::FirstSpawnZombie()
{
	// 서버에서만 실행이 되어야 한다.
	if (!HasAuthority())
	{
		return;
	}

	if (IsValid(SpawnClass) && SpawnNumber > 0)
	{
		for (int32 i = 0; i < SpawnNumber; ++i)
		{
			//랜덤 스폰 위치를 생성한다.
			FVector SpawnLoc = FVector::ZeroVector;
			CalcRandomSpawnLoc(SpawnLoc);

			// Monster CDO를 얻어온다.
			TObjectPtr<AZombieBase>	CDO = SpawnClass->GetDefaultObject<AZombieBase>();
			if (IsValid(CDO))
			{
				SpawnLoc.Z += CDO->GetHalfHeight();
			}

			//좀비를 스폰시킨다.
			FActorSpawnParameters	param;
			param.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AZombieBase* SpawnZombie = GetWorld()->SpawnActor<AZombieBase>(SpawnClass,
				SpawnLoc, GetActorRotation(), param);

			SpawnZombie->SetSpawnPointLocation(GetActorLocation());
		}
	}
}

void AZombieSpawnPoint::SpawnZombie()
{
	// 서버에서만 실행이 되어야 한다.
	if (!HasAuthority())
	{
		return;
	}

	if (IsValid(SpawnClass) && SpawnNumber > 0)
	{
		for (int32 i = 0; i < SpawnNumber; ++i)
		{
			//랜덤 스폰 위치를 생성한다.
			FVector SpawnLoc = FVector::ZeroVector;
			CalcRandomSpawnLoc(SpawnLoc);

			// Monster CDO를 얻어온다.
			TObjectPtr<AZombieBase>	CDO = SpawnClass->GetDefaultObject<AZombieBase>();
			if (IsValid(CDO))
			{
				SpawnLoc.Z += CDO->GetHalfHeight();
			}

			//좀비를 스폰시킨다.
			FActorSpawnParameters	param;
			param.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			AZombieBase* SpawnZombie = GetWorld()->SpawnActor<AZombieBase>(SpawnClass,
				SpawnLoc, GetActorRotation(), param);

			//첫번째 좌표 배열에서 랜덤으로 좌표를 하나 가져온다.
			FVector FirstDstLoc = FVector::ZeroVector;
			CalcRandomFirstDst(FirstDstLoc);

			//스폰된 좀비의 블랙보드에 집결지 이동 불리언, 좌표를 넣어준다.
			AZombieAIController* Ctrl = SpawnZombie->GetAICtrl();
			if (IsValid(Ctrl))
			{
				UBlackboardComponent* BB = Ctrl->GetBlackboardComponent();
				if (IsValid(BB))
				{
					BB->SetValueAsBool(TEXT("IsNeedSpawnMoving"), true);
					BB->SetValueAsVector(TEXT("SpawnMovingDstLoc"), FirstDstLoc);
				}
			}
		}
	}
}

bool AZombieSpawnPoint::CalcRandomSpawnLoc(FVector& Loc)
{
	//내비 메시 볼륨은 서버에만 있으므로 클라에서는 동작하지 못하도록 해야 한다.
	if (!HasAuthority())
	{
		return false;
	}

	//이동 가능한 무작위 위치를 만들어주는 함수를 호출하기 위해 네비게이션 시스템을 얻어온다.
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	//생성된 위치를 담을 구조체
	FNavLocation ResultLoc;

	//성공했다면 만든 좌표를, 실패했다면 현재 위치를 다음 이동위치로 지정한다.
	bool Result = NavSystem->GetRandomReachablePointInRadius(GetActorLocation(), SpawnRadius, ResultLoc);
	if (Result)
	{
		Loc = ResultLoc.Location;
	}
	else
	{
		Loc = GetActorLocation();
	}

	return true;
}

bool AZombieSpawnPoint::CalcRandomFirstDst(FVector& Loc)
{
	//서버만 스폰에 관여해야 한다.
	if (!HasAuthority())
	{
		return false;
	}

	// 스플라인의 포인트 수를 얻어온다.
	int32	Count = FirstDstList->GetNumberOfSplinePoints();
	//랜덤으로 인덱스를 하나 만든다.
	int32 RandIndex = FMath::RandRange(0, Count - 1);
	FVector FirstLoc = FirstDstPointArray[RandIndex];

	Loc = FirstLoc;

	return true;
}

