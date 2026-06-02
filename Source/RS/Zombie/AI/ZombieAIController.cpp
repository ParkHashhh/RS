// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"

AZombieAIController::AZombieAIController()
{
	//인공지능에 사용할 감각 컴포넌트를 생성한다.
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SetPerceptionComponent(*AIPerception);

	//시각을 사용하도록 생성한다.
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight"));

	//탐지 대상을 지정한다
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	//시각을 사용하도록 등록하고 시각을 주 감각으로 지정한다.
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	SetGenericTeamId(FGenericTeamId(TeamZombie));
}

void AZombieAIController::SetBT(UBehaviorTree* BehaviorTree)
{
	BT = BehaviorTree;
}

void AZombieAIController::SetDetectRange_Sight(float Range)
{
	//어차피 탐지에 실패했을 때, 타겟을 제거하는 기능을 없애버릴 예정이므로 상실 거리는 설정할 필요가 없다.
	//내부 로직 때문에 혹시 모를 오류를 방지하기 위해 둘을 같게 설정했다.
	SightConfig->SightRadius = Range;
	SightConfig->LoseSightRadius = Range;
	AIPerception->ConfigureSense(*SightConfig);
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
