// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Wander.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/Zombie/ZombieBase.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/CapsuleComponent.h"


UBTTask_Wander::UBTTask_Wander()
{
	NodeName = TEXT("Wander");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	if (!IsValid(Ctrl))
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = Ctrl->GetBlackboardComponent();
	if (!IsValid(BlackboardComp))
	{
		return EBTNodeResult::Failed;
	}

	//배회는 타겟이 존재하면 그만하고 전투상태로 넘어가야 한다.
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (IsValid(Target))
	{
		return EBTNodeResult::Succeeded;
	}

	AZombieBase* Zombie = Ctrl->GetPawn<AZombieBase>();
	if (!IsValid(Zombie))
	{
		return EBTNodeResult::Failed;
	}

	EPathFollowingRequestResult::Type MoveRequestResult = Ctrl->MoveToLocation(Zombie->GetNextPointLocation());
	if (MoveRequestResult == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}
	Zombie->ChangeAnim(EZombieAnimType::Walk);

	return EBTNodeResult::InProgress;
}

void UBTTask_Wander::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	if (!IsValid(Ctrl))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	UBlackboardComponent* BlackboardComp = Ctrl->GetBlackboardComponent();
	if (!IsValid(BlackboardComp))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	//타겟이 있으면 중단해야 한다는 점에 한번 더 유의해야 한다.
	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (IsValid(Target))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	AZombieBase* Zombie = Ctrl->GetPawn<AZombieBase>();
	if (!IsValid(Zombie))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	EPathFollowingStatus::Type MoveResult = Ctrl->GetMoveStatus();
	if (MoveResult == EPathFollowingStatus::Idle)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	//목표 지점에 도착했는지 확인한다.
	FVector TargetLoc, ZombieLoc;

	TargetLoc = Zombie->GetNextPointLocation();
	ZombieLoc = Zombie->GetActorLocation();

	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Zombie->GetRootComponent());
	if (Capsule)
	{
		ZombieLoc.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}

	float Distance = FVector::Dist(TargetLoc, ZombieLoc);
	//5.0 이내의 거리라면 도착한 것으로 간주한다.
	if (Distance <= 5.f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

void UBTTask_Wander::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	Ctrl->StopMovement();
	AZombieBase* Zombie = Ctrl->GetPawn<AZombieBase>();
	if (Zombie)
	{
		Zombie->CalcNextPointLocation();
		Zombie->ChangeAnim(EZombieAnimType::Idle);
	}
}
