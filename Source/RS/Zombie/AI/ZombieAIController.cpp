// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../ZombieBase.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/LogCategory.h"
#include "Navigation/PathFollowingComponent.h"
#include "RS/Zombie/ZombieBase.h"

AZombieAIController::AZombieAIController()
{
	//인공지능에 사용할 감각 컴포넌트를 생성한다.
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SetPerceptionComponent(*AIPerception);

	//시각을 사용하도록 생성한다.
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));

	//테스트용 임시 코드
	SightConfig->SightRadius = 1.f;
	SightConfig->LoseSightRadius = 1.f;
	SightConfig->PeripheralVisionAngleDegrees = 160.f;

	//탐지 대상을 지정한다
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	//시각을 사용하도록 등록하고 시각을 주 감각으로 지정한다.
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	SetGenericTeamId(FGenericTeamId(TeamMonster));

	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AZombieAIController::OnTarget);
}
void AZombieAIController::BeginPlay()
{
	Super::BeginPlay();
	AIPerception->RequestStimuliListenerUpdate();
}

void AZombieAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UPathFollowingComponent* PFComp = GetPathFollowingComponent();
	UBlackboardComponent* BB = GetBlackboardComponent();
	AZombieBase* Zombie = GetPawn<AZombieBase>();

	if (!PFComp || !BB || !Zombie)
	{
		return;
	}

	//내비링크 이용을 시작하면 불리언 변수를 활성화하고 도착 지점을 백업한다.
	if (PFComp && PFComp->IsCurrentSegmentNavigationLink())
	{
		//내비링크 사용중 변수를 on 한다.
		GetBlackboardComponent()->SetValueAsBool(TEXT("IsUsingNavLink"), true);

		//현재 내비 데이터로부터 좌표 배열을 가져온다.
		FNavigationPath* PathData = PFComp->GetPath().Get();
		TArray<FNavPathPoint> PathPointArray = PathData->GetPathPoints();

		//배열에서 내비링크 도착점 좌표를 백업한다.
		int EndIndex = PFComp->GetNextPathIndex();
		EndLoc = PathPointArray[EndIndex].Location;
	}

	bool bIsUsingNavLink = BB->GetValueAsBool(TEXT("IsUsingNavLink"));

	//불리언이 true인데 내비링크 사용중이 false라고 뜬다면 길이 막혀서 강제로 꺼진 것이다.
	if (PFComp && bIsUsingNavLink && !PFComp->IsCurrentSegmentNavigationLink())
	{
		StuckTime += DeltaTime;

		if (StuckTime >= 0.5f)
		{
			StuckTime = 0.f;

			//좀비를 내비링크 도착 지점으로 보낸다.
			float UpLocScale = Zombie->GetHalfHeight();
			FVector EscapeLoc = EndLoc + FVector(0.0, 0.0, UpLocScale);
			Zombie->SetActorLocation(EscapeLoc);
			BB->SetValueAsBool(TEXT("IsUsingNavLink"), false);
			StopMovement();
		}
	}

	//내비링크 사용을 무사히 마친 경우 : 내비링크 사용중인데 착지-추적태스크 발동으로 이동이 시작된 경우
	if (bIsUsingNavLink && !PFComp->IsCurrentSegmentNavigationLink() && GetMoveStatus() == EPathFollowingStatus::Moving)
	{
		//내비링크를 정상적으로 이용한 경우
		StuckTime = 0.f;
		EndLoc = FVector::ZeroVector;
		BB->SetValueAsBool(TEXT("IsUsingNavLink"), false);
	}
}

void AZombieAIController::SetDetectRange_Sight(float Range)
{
	//어차피 탐지에 실패했을 때, 타겟을 제거하는 기능을 없애버릴 예정이므로 상실 거리는 설정할 필요가 없다.
	//내부 로직 때문에 혹시 모를 오류를 방지하기 위해 둘을 같게 설정했다.
	SightConfig->SightRadius = Range;
	SightConfig->LoseSightRadius = Range;
	AIPerception->ConfigureSense(*SightConfig);
}

void AZombieAIController::OnTarget(AActor* Actor, FAIStimulus Stimulus)
{
	if (!IsValid(BT) || !Blackboard)
		return;

	AZombieBase* Zombie = Cast<AZombieBase>(GetPawn());
	if (!IsValid(Zombie))
		return;

	AActor* Target = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));

	//현재 블랙보드에 타겟이 없을 경우에만 타겟으로 지정한다.
	if (Stimulus.WasSuccessfullySensed() && !IsValid(Target))
	{
		Blackboard->SetValueAsObject(TEXT("Target"), Actor);
	}
	else
	{
		//이후 추적 해제 기능이 필요하다면 이곳에서 타겟을 해제해준다.
	}
}

ETeamAttitude::Type AZombieAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);

	if (!OtherTeamAgent)
		return ETeamAttitude::Neutral;

	else if (OtherTeamAgent->GetGenericTeamId().GetId() == TeamNeutral)
		return ETeamAttitude::Neutral;

	return GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId() ?
		ETeamAttitude::Friendly : ETeamAttitude::Hostile;
}

void AZombieAIController::ActivateBT()
{
	if (!IsValid(BT))
	{
		RS_PRINT_SCREEN(30.f, FColor::Red, TEXT("NO BT!"));
		return;
	}

	if (!RunBehaviorTree(BT))
	{
		RS_PRINT_SCREEN(30.f, FColor::Red, TEXT("BT Run Failed!"));
		return;
	}
}
