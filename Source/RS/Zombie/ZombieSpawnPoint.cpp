// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpawnPoint.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "ZombieBase.h"
#include "NavigationSystem.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/GameMode/MainGameMode.h"
#include "RS/Zombie/ZombieManager.h"
#include "Components/BoxComponent.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"

// Sets default values
AZombieSpawnPoint::AZombieSpawnPoint()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	FirstDstList = CreateDefaultSubobject<USplineComponent>(TEXT("FirstDstList"));
	DetectArea = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectArea"));

	SetRootComponent(Root);

	FirstDstList->SetupAttachment(Root);
	DetectArea->SetupAttachment(Root);

	DetectArea->SetBoxExtent(FVector(1500.0, 1500.0, 1000.0));

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

	//감지 박스에 플레이어가 오버랩/언오버랩 되었을 경우 불리언을 바꿔주기 위해 이벤트를 연결한다.
	if (IsValid(DetectArea))
	{
		DetectArea->OnComponentBeginOverlap.AddDynamic(this, &AZombieSpawnPoint::PlayerInArea);
		DetectArea->OnComponentEndOverlap.AddDynamic(this, &AZombieSpawnPoint::PlayerOutArea);

		FTimerHandle TempHandle;
		GetWorld()->GetTimerManager().SetTimer(TempHandle, FTimerDelegate::CreateUObject(this, &AZombieSpawnPoint::CheckStartOverlap), 0.1f, false);
	}

	//디폴트 스폰 설정이 되어 있는 경우에만 처음에 스폰을 한다.
	if (bDefaultSpawn)
	{
		FirstSpawnZombie();
	}

	//생성된 스폰 포인트를 매니저의 스폰 포인트 배열에 등록한다.
	AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	UZombieManager* ZombieMgr = GameMode->GetZombieManager();
	ZombieMgr->SpawnPointRegister(this);
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

	if (IsValid(SpawnClass) && SpawnNumber > 0 && bSpawnActivateByMaxCount)
	{
		AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
		UZombieManager* ZombieMgr = GameMode->GetZombieManager();

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

			//좀비가 생성되면 매니저의 좀비 배열에 등록한다.
			ZombieMgr->ZombieRegister(SpawnZombie);

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

	AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	UZombieManager* ZombieMgr = GameMode->GetZombieManager();

	if (IsValid(SpawnClass) && SpawnNumber > 0 && bSpawnActivateByMaxCount)
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

			//좀비가 생성되면 매니저의 좀비 배열에 등록한다.
			ZombieMgr->ZombieRegister(SpawnZombie);

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

void AZombieSpawnPoint::PlayerInArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AddPlayerCount(OtherActor);
}

void AZombieSpawnPoint::PlayerOutArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APlayerCharacterBase* DetectedActor = Cast<APlayerCharacterBase>(OtherActor);
	if (IsValid(DetectedActor))
	{
		--PlayerNumInArea;

		//만약의 상황을 대비해 0 이하로는 가지 못하게 한다.
		if (PlayerNumInArea < 0)
		{
			PlayerNumInArea = 0;
		}

		//영역 내의 플레이어가 0명이 되면 타이머를 Clear 한다.
		if (PlayerNumInArea == 0)
		{
			GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		}
	}
}

void AZombieSpawnPoint::AddPlayerCount(AActor* OtherActor)
{
	APlayerCharacterBase* DetectedActor = Cast<APlayerCharacterBase>(OtherActor);
	if (IsValid(DetectedActor))
	{
		++PlayerNumInArea;

		//만약의 상황을 대비해 최대 플레이어 수 이상으로는 가지 못하게 한다.
		if (PlayerNumInArea > 4)
		{
			PlayerNumInArea = 4;
		}

		//무한 스폰을 위한 타이머를 세팅하는 조건
		//1. 플레이어 수가 0을 초과
		//2. 무한스폰용 스폰포인트 설정이 되어 있어야 함
		//3. 스폰 타임을 지정해 두었어야 함.
		//4. 타이머가 설정되지 않은 상태여야 함.
		if (PlayerNumInArea > 0)
		{
			//타이머를 이용해 무한 스폰하는 스폰 포인트인 경우 타이머를 설정한다.
			if (bInfiniteSpawnUseTimer && SpawnInterval > 0.f && !GetWorld()->GetTimerManager().IsTimerActive(SpawnTimer))
			{
				GetWorld()->GetTimerManager().SetTimer(SpawnTimer,
					FTimerDelegate::CreateUObject(this, &AZombieSpawnPoint::SpawnZombie), SpawnInterval, true, 3.f);
			}
		}
	}
}

void AZombieSpawnPoint::CheckStartOverlap()
{
	if (IsValid(DetectArea))
	{
		//플레이어의 스폰 지점이 영역 안에 존재하면 더하기 이벤트가 호출되지 않으므로 한번 강제 체크를 한다.
		TArray<AActor*> OverlapingActorArray;
		DetectArea->GetOverlappingActors(OverlapingActorArray);
		for (AActor* Actor : OverlapingActorArray)
		{
			AddPlayerCount(Actor);
		}
	}
}

void AZombieSpawnPoint::SpawnZombieByBossTrigger(AActor* Target)
{
	// 서버에서만 실행이 되어야 한다.
	if (!HasAuthority())
	{
		return;
	}

	AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	UZombieManager* ZombieMgr = GameMode->GetZombieManager();

	if (IsValid(SpawnClass) && SpawnNumber > 0 && bSpawnActivateByMaxCount)
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

			//좀비가 생성되면 매니저의 좀비 배열에 등록한다.
			ZombieMgr->ZombieRegister(SpawnZombie);

			SpawnZombie->SetSpawnPointLocation(GetActorLocation());

			//타겟을 바로 넣어주니까 애님 인스턴스 연결이 안되서 작동을 하지 않는다.
			//잠깐 기다렸다가 타겟을 넣어준다.
			FTimerHandle DelayTimerHandle;
			GetWorld()->GetTimerManager().SetTimer(DelayTimerHandle, [SpawnZombie, Target]()
				{
					//보스가 소환한 좀비는 타겟을 바로 넣어준다.
					AZombieAIController* SpawnedZCtrl = SpawnZombie->GetAICtrl();
					if (IsValid(SpawnedZCtrl))
					{
						UBlackboardComponent* SpawnedZBB = SpawnedZCtrl->GetBlackboardComponent();
						if (IsValid(SpawnedZBB))
						{
							SpawnedZBB->SetValueAsObject(TEXT("Target"), Target);
						}
					}
				},
				0.2f, false);
		}
	}
}

void AZombieSpawnPoint::SetSpawnActivation(bool IsActivate)
{
	bSpawnActivateByMaxCount = IsActivate;
}

