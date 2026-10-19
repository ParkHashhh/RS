// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_TraceNoneBoss.h"
#include "AIController.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "../ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "RS/Core/RSTypes.h"
#include "NavigationSystem.h"
#include "RS/Player/Character/PlayerCharacterBase.h"

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

	AZombieBase* Zombie = Ctrl->GetPawn<AZombieBase>();
	if (!IsValid(Zombie))
	{
		return EBTNodeResult::Failed;
	}

	//만약 타겟이 있는데 타겟이 죽은 상태라면 마찬가지로 동작하지 못하게 해야 한다.
	APlayerCharacterBase* CastedTarget = Cast<APlayerCharacterBase>(Target);
	if (!IsValid(CastedTarget) || CastedTarget->IsDead())
	{
		BlackboardComp->SetValueAsObject(TEXT("Target"), nullptr);
		return EBTNodeResult::Failed;
	}

	//만약 내비링크를 사용중이라면 자체 이동중이므로 이동 명령을 내리지 않는다.
	bool bIsUsingNavLink = BlackboardComp->GetValueAsBool(TEXT("IsUsingNavLink"));
	if (bIsUsingNavLink)
	{
		return EBTNodeResult::InProgress;
	}

	//이미 추적 중단 거리 안이라면 추적을 할 필요가 없다.
	float Distance = Zombie->CalcDistanceToTarget(Target);
	float TraceEndRange = BlackboardComp->GetValueAsFloat(TEXT("TraceEndDistance"));
	if (Distance < TraceEndRange)
	{
		return EBTNodeResult::Failed;
	}

	//좀비를 이동시킨다.
	EPathFollowingRequestResult::Type MoveRequestResult = Ctrl->MoveToActor(Target, -1.f, false, true, false);
	if (MoveRequestResult == EPathFollowingRequestResult::Failed)
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

	//만약 타겟이 있는데 타겟이 죽은 상태라면 마찬가지로 동작하지 못하게 해야 한다.
	APlayerCharacterBase* CastedTarget = Cast<APlayerCharacterBase>(Target);
	if (!IsValid(CastedTarget) || CastedTarget->IsDead())
	{
		BlackboardComp->SetValueAsObject(TEXT("Target"), nullptr);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	//좀비가 내비링크를 사용 중이라면 사용을 종료할때까지 기다린다.
	AZombieAIController* ZombieCtrl = Cast<AZombieAIController>(Ctrl);
	if (!Ctrl)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	bool bIsUsingNavLink = BlackboardComp->GetValueAsBool(TEXT("IsUsingNavLink"));
	bool bIsJumpEnd = ZombieCtrl->GetIsJumpEndNow();
	if (bIsUsingNavLink || bIsJumpEnd)
	{
		return;
	}

	//좀비가 내비메쉬 안인지 체크한다.
	bool bIsZombieIn = false;
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (IsValid(NavSys))
	{
		FNavLocation ZCurrentLoc;
		bIsZombieIn = NavSys->ProjectPointToNavigation(Zombie->GetActorLocation(), ZCurrentLoc, FVector(10.f, 10.f, 200.f));
	}

	//좀비가 내비메쉬 영역 안에 있을 때에만
	if (bIsZombieIn)
	{
		//타겟이 사거리 안으로 들어왔다면 공격 태스크로 진입한다.
		float Distance = Zombie->CalcDistanceToTarget(Target);
		float TraceEndRange = BlackboardComp->GetValueAsFloat(TEXT("TraceEndDistance"));

		if (Distance <= TraceEndRange)
		{
			BlackboardComp->SetValueAsBool(TEXT("AttackPossible"), true);
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}

	//플레이어가 내비메쉬 밖으로 나갔는지 체크한다.
	if (IsValid(NavSys))
	{
		FNavLocation TCurrentLoc;
		bool bIsTargetIn = NavSys->ProjectPointToNavigation(Target->GetActorLocation(), TCurrentLoc, FVector(100.f, 100.f, 200.f));
		if (!bIsTargetIn)
		{
			//플레이어가 만약 내비메쉬 밖으로 나가면
			//일단 비전투 상태로 만든다 - 추후 수정해야 함
			Ctrl->StopMovement();
			BlackboardComp->SetValueAsObject(TEXT("Target"), nullptr);
			BlackboardComp->SetValueAsBool(TEXT("AttackPossible"), false);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}
}

void UBTTask_TraceNoneBoss::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	Ctrl->StopMovement();

	AZombieBase* Zombie = Ctrl->GetOwner<AZombieBase>();
	if (IsValid(Zombie))
	{
		Zombie->ChangeAnim(EZombieAnimType::CombatIdle);
	}
}

