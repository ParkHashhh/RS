// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "../ZombieBase.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/LogCategory.h"
#include "Navigation/PathFollowingComponent.h"
#include "RS/Zombie/ZombieBase.h"
#include "GameFramework/FloatingPawnMovement.h"

AZombieAIController::AZombieAIController()
{
	//인공지능에 사용할 감각 컴포넌트를 생성한다.
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SetPerceptionComponent(*AIPerception);

	//시각을 사용하도록 생성한다.
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("Hearing"));

	//초기에 값이 오염되어 바로 포착되는 것을 방지한다.
	SightConfig->SightRadius = 1.f;
	SightConfig->LoseSightRadius = 1.f;
	SightConfig->PeripheralVisionAngleDegrees = 160.f;

	//시각의 탐지 대상을 지정한다
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	//초기에 값이 오염되어 바로 포착되는 것을 방지한다.
	HearingConfig->HearingRange = 1.f;

	//청각의 탐지 대상을 지정한다
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

	//시각을 사용하도록 등록하고 시각을 주 감각으로 지정한다.
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	AIPerception->ConfigureSense(*HearingConfig);

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
	UFloatingPawnMovement* FPM = Zombie->GetMovement();

	if (!PFComp || !BB || !Zombie || !FPM)
	{
		return;
	}

	bool bIsUsingNavLink = BB->GetValueAsBool(TEXT("IsUsingNavLink"));

	//불리언이 false인데 내비링크 사용중이 true가 되면 내비링크 사용을 시작한 것이다.
	//내비링크 이용을 시작하면 불리언 변수를 활성화하고 도착 지점을 백업한다.
	if (PFComp && !bIsUsingNavLink && PFComp->IsCurrentSegmentNavigationLink())
	{
		//내비링크 사용중 변수를 on 한다.
		GetBlackboardComponent()->SetValueAsBool(TEXT("IsUsingNavLink"), true);

		//현재 내비 데이터로부터 좌표 배열을 가져온다.
		FNavigationPath* PathData = PFComp->GetPath().Get();
		TArray<FNavPathPoint> PathPointArray = PathData->GetPathPoints();

		//배열에서 내비링크 도착점 좌표를 백업한다.
		int32 EndIndex = PFComp->GetNextPathIndex();
		EndLoc = PathPointArray[EndIndex].Location;

		//도약 모션을 위한 일시 정지
		PFComp->PauseMove();

		//애니메이션을 점프로 바꾼다.
		Zombie->ChangeAnim(EZombieAnimType::Jump);

		//좀비의 이전 속도를 백업한다.
		PrevSpeed = FPM->MaxSpeed;

		FPM->MaxSpeed += 300.f;

		return;
	}

	//불리언이 true인데 내비링크 사용중이 false라고 뜬다면 추가 검사가 필요하다.
	if (PFComp && bIsUsingNavLink && !PFComp->IsCurrentSegmentNavigationLink())
	{
		//좀비와 도착점의 거리에 따라 처리해야 할 내용이 달라진다.
		//1. 좀비가 도착점에 도착하지 않은 경우 : 고장난것이다.
		float Distance = FVector::Dist2D(Zombie->GetActorLocation(), EndLoc);

		if (Distance > 10.f)
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

				//착지 모션을 위한 일시 정지
				PFComp->PauseMove();

				//착지 섹션으로 몽타주를 변경한다.
				Zombie->SetIsJumpEnd(true);

				//백업해둔 속도를 적용한다.
				FPM->MaxSpeed = PrevSpeed;
			}
		}
		//거리가 충분히 가까운 경우 : 도착한것이다.
		else
		{
			//착지 모션을 위한 일시 정지
			PFComp->PauseMove();

			//착지 섹션으로 몽타주를 변경한다.
			Zombie->SetIsJumpEnd(true);

			//백업해둔 속도를 적용한다.
			FPM->MaxSpeed = PrevSpeed;

			//사용한 변수들을 초기화한다.
			StuckTime = 0.f;
			EndLoc = FVector::ZeroVector;
			BB->SetValueAsBool(TEXT("IsUsingNavLink"), false);
		}
	}

	if (PFComp->IsCurrentSegmentNavigationLink())
	{
		if (FPM->Velocity.Z < 0.f)
		{
			FPM->MaxSpeed += DeltaTime * 600.f;
		}
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

void AZombieAIController::SetDetectRange_Hearing(float Range)
{
	HearingConfig->HearingRange = Range;
	AIPerception->ConfigureSense(*HearingConfig);
}

void AZombieAIController::OnTarget(AActor* Actor, FAIStimulus Stimulus)
{
	if (!IsValid(BT) || !Blackboard)
		return;

	AZombieBase* Zombie = Cast<AZombieBase>(GetPawn());
	if (!IsValid(Zombie))
		return;

	AActor* Target = Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(TEXT("Target")));

	//감지에 성공함
	if (Stimulus.WasSuccessfullySensed())
	{
		//현재 타겟을 감지한 감각이 무엇인지 확인
		EDetectPerceptionType CurrentType = EDetectPerceptionType::None;
		if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Sight>())
		{
			CurrentType = EDetectPerceptionType::Sight;
		}
		else if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Hearing>())
		{
			CurrentType = EDetectPerceptionType::Hearing;
		}

		//현재 타겟이 있는지 확인
		if (!IsValid(Target))
		{
			//없으면 타겟으로 지정 + 어떤 감각으로 감지했는지 저장
			Blackboard->SetValueAsObject(TEXT("Target"), Actor);
			TargetPerceptionType = CurrentType;
		}
		else
		{
			//현재 타겟이 있다면 그 타겟이 어떤 감각으로 감지한 타겟인지 확인
			if (TargetPerceptionType == EDetectPerceptionType::Sight)
			{
				//이전 타겟을 시각으로 감지한 경우에는 전부 무시한다. (타겟을 바꾸지 않는다.)
				return;
			}
			else if (TargetPerceptionType == EDetectPerceptionType::Hearing)
			{
				//이전 타겟을 청각으로 감지했다면 현재 타겟이 어떤 감각으로 감지되었는지 확인
				if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Sight>())
				{
					//현재 타겟이 시각으로 감지한 타겟이라면 타겟을 변경한다.
					Blackboard->SetValueAsObject(TEXT("Target"), Actor);
					TargetPerceptionType = CurrentType;
				}
				else if (Stimulus.Type == UAISense::GetSenseID<UAISenseConfig_Sight>())
				{
					//현재 타겟이 청각으로 감지한 타겟이라면 무시한다.
					return;
				}
			}
		}
	}
	//감지에 실패함
	else
	{
		//좀비가 추적에 실패하면 타겟을 지워야 하는 경우 이곳에서 처리한다.
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

void AZombieAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
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
