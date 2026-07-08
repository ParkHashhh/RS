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

	//다시 대기 상태로 돌아왔다는 것은 플레이어가 죽어서 타겟이 사라졌다는 의미이다.
	Zombie->ChangeSpeed_Walk();

	//대기와 포효 중 하나를 선택한다.
	double RandPercent = FMath::RandRange(0.0, 100.0);
	//포효 애니메이션을 가지고 있는지를 체크한다.
	bool HasRoarAnim = Zombie->CheckRoar();
	if (HasRoarAnim && RandPercent <= 10.0)
	{
		//포효를 한다.
		Zombie->ChangeAnim(EZombieAnimType::Roar);
		BlackboardComp->SetValueAsBool(TEXT("ActivateRoar"), true);
		BlackboardComp->SetValueAsBool(TEXT("IsRoar"), true);
	}
	else
	{
		//대기를 한다.
		//1. 애니메이션을 Idle로 바꾼다.
		Zombie->ChangeAnim(EZombieAnimType::Idle);

		//2. 타이머를 세팅한다.
		FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
		Timer->Finish = false;
		float WaitTime = FMath::RandRange(3.f, 7.f);

		OwnerComp.GetWorld()->GetTimerManager().SetTimer(Timer->WaitTimer,
			FTimerDelegate::CreateUObject(this, &UBTTask_WaitCustom::WaitFinish, NodeMemory), WaitTime, false);
	}

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

	bool ActivateRoar = BlackboardComp->GetValueAsBool(TEXT("ActivateRoar"));
	if (ActivateRoar)
	{
		//포효를 했다면 포효가 끝났는지를 확인한다.
		bool IsRoar = BlackboardComp->GetValueAsBool(TEXT("IsRoar"));
		if (!IsRoar)
		{
			//포효가 끝났다면 대기 태스크를 종료한다.
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
		else
		{
			//포효가 아직 끝나지 않았다면 이번 프레임은 그냥 넘긴다.
			return;
		}
	}
	else
	{
		//틱마다 타이머 종료 여부를 체크한다.
		FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
		if (Timer->Finish)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}
}

void UBTTask_WaitCustom::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = Ctrl->GetBlackboardComponent();
	bool ActivateRoar = BlackboardComp->GetValueAsBool(TEXT("ActivateRoar"));
	//좀비가 대기시간 동안 어떤 행동을 했느냐에 따라 정리할 내용이 달라진다.
	if (ActivateRoar)
	{
		//포효했다면 변수를 초기화한다.
		BlackboardComp->SetValueAsBool(TEXT("ActivateRoar"), false);
	}
	else
	{
		//대기했다면 타이머를 청소한다.
		FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
		if (Timer->WaitTimer.IsValid())
		{
			OwnerComp.GetWorld()->GetTimerManager().ClearTimer(Timer->WaitTimer);
		}
	}
}

void UBTTask_WaitCustom::WaitFinish(uint8* NodeMemory)
{
	//타이머가 종료되면 종료를 알리는 불 변수를 true로 바꾼다.
	FWaitTimer* Timer = (FWaitTimer*)NodeMemory;
	Timer->Finish = true;
}
