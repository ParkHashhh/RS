// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_WaitCustom.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/Zombie/ZombieBase.h"


UBTTask_WaitCustom::UBTTask_WaitCustom()
{
	NodeName = TEXT("WaitCustom");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

uint16 UBTTask_WaitCustom::GetInstanceMemorySize()	const
{
	return sizeof(FWaitTimer);
}

EBTNodeResult::Type UBTTask_WaitCustom::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
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

	//1. 애니메이션을 Idle로 바꾼다.
	Zombie->ChangeAnim(EZombieAnimType::Idle);

	//2. 타이머를 세팅한다.
	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
	Timer->Finish = false;
	float WaitTime = 1.f;

	OwnerComp.GetWorld()->GetTimerManager().SetTimer(Timer->WaitTimer,
		FTimerDelegate::CreateUObject(this, &UBTTask_WaitCustom::WaitFinish, NodeMemory), WaitTime, false);

	return EBTNodeResult::InProgress;
}
void UBTTask_WaitCustom::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
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

	//틱마다 타이머 종료 여부를 체크한다.
	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
	if (Timer->Finish)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

void UBTTask_WaitCustom::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	//태스크가 종료되면 타이머를 제거한다.
	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
	if (Timer->WaitTimer.IsValid())
	{
		OwnerComp.GetWorld()->GetTimerManager().ClearTimer(Timer->WaitTimer);
	}
}

void UBTTask_WaitCustom::WaitFinish(uint8* NodeMemory)
{
	//타이머가 종료되면 종료를 알리는 불 변수를 true로 바꾼다.
	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
	Timer->Finish = true;
}
