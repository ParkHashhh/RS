// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TraceNoneBoss.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "../ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "RS/RSTypes.h"

UBTTask_TraceNoneBoss::UBTTask_TraceNoneBoss()
{
	NodeName = TEXT("Trace");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_TraceNoneBoss::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	//방어 구문을 작성해야 한다.
	//1. 컨트롤러가 유효한가
	//2. 그 안의 블랙보드가 유효한가
	//3. 그 안에 타겟 정보가 유효한가
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

	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (!IsValid(Target))
	{
		return EBTNodeResult::Failed;
	}

	//타겟까지 유효하다면 이동한다.
	EPathFollowingRequestResult::Type MoveRequestResult = Ctrl->MoveToActor(Target);
	if (MoveRequestResult == EPathFollowingRequestResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	AZombieBase* Zombie = Ctrl->GetPawn<AZombieBase>();
	if (!IsValid(Zombie))
	{
		return EBTNodeResult::Failed;
	}

	//좀비가 유효하므로 애니메이션을 변경한다.
	Zombie->ChangeAnim(EZombieAnimType::Run);

	//추적중이므로 이동을 완료할 때 까지는 현상유지를 해야 한다.
	//이 동안은 태스크의 Tick이 수행된다.
	return EBTNodeResult::InProgress;
}

void UBTTask_TraceNoneBoss::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	//방어 구문을 작성한다.
	//1. 컨트롤러가 유효한가
	//2. 그 안의 블랙보드가 유효한가
	//3. 그 안의 타겟이 유효한가
	//4. 좀비가 유효한가.

	//셀렉터이므로 성공이 반환되면 전투상태가 종료된다.
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

	AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("Target")));
	if (!IsValid(Target))
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

	//현재 움직임 상태를 체크한다
	//대기중 이라면 추적을 완료하고 타겟 앞에 도달했다고 판단할 수 있다.
	EPathFollowingStatus::Type MoveResult = Ctrl->GetMoveStatus();
	if (MoveResult == EPathFollowingStatus::Idle)
	{
		//다음 전투 태스크를 수행해야 하므로 실패를 반환한다.
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	//이곳은 추적중인 상태이므로 추적을 계속 해야할지를 결정해야 한다.
	//두 액터의 캡슐 중점을 지면으로 맞춘 후 거리를 계산한다.
	FVector TargetLoc, ZombieLoc;

	TargetLoc = Target->GetActorLocation();
	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(Target->GetRootComponent());
	if (Capsule)
	{
		TargetLoc.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}

	ZombieLoc = Zombie->GetActorLocation();
	Capsule = Cast<UCapsuleComponent>(Zombie->GetRootComponent());
	if (Capsule)
	{
		ZombieLoc.Z -= Capsule->GetScaledCapsuleHalfHeight();
	}

	float Distance = FVector::Dist(TargetLoc, ZombieLoc);

	//몬스터의 어트리뷰트 셋으로부터 몬스터의 사거리를 얻어와야 한다.
	UBaseAttributeSet* ZombieAttri = Zombie->GetAttributeSet();
	float AttackRange = ZombieAttri->GetAttackDistance();

	if (Distance <= AttackRange)
	{
		//추적을 통해 사거리 내로 접근했으므로 다음 전투 태스크를 진행한다.
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}
void UBTTask_TraceNoneBoss::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	Ctrl->StopMovement();
}

