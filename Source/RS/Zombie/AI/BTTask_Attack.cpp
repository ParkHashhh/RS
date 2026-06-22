// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "../../GAS/BaseAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"
#include "RS/LogCategory.h"


UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
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

	//현재 추적중인 대상이 공격 타겟이 된다.
	BlackboardComp->SetValueAsObject(TEXT("AttackTarget"), Target);

	//좀비의 애니메이션을 변경한다.
	Zombie->ChangeAnim(EZombieAnimType::Attack);
	//블랙보드의 '공격중' 변수를 true로 바꾼다.
	BlackboardComp->SetValueAsBool(TEXT("Attacking"), true);

	return EBTNodeResult::InProgress;
}
void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
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

	float Distance = Zombie->CalcDistanceToTarget(Target);

	UBaseAttributeSet* ZombieAttri = Zombie->GetAttributeSet();
	float AttackRange = ZombieAttri->GetAttackDistance();

	bool IsAttacking = BlackboardComp->GetValueAsBool(TEXT("Attacking"));

	if (!IsAttacking)
	{
		if (Distance < AttackRange)
		{
			//공격이 끝났을 때 여전히 사거리 안이라면 다음 공격을 위해 좀비의 방향을 돌려준다.
			//공격이 끝났을 때 사거리 밖이라면 추적 태스크에서 이동을 할 것이므로 자연스럽게 방향이 돌아간다.
			//고개를 돌릴 때 보간과 걷기 애니메이션을 통해 순간이동처럼 보이는 것을 완화시킬 예정
			FVector ZombieLoc = Zombie->GetActorLocation();
			FVector TargetLoc = Target->GetActorLocation();
			ZombieLoc.Z = TargetLoc.Z;

			FRotator Rot = UKismetMathLibrary::FindLookAtRotation(ZombieLoc, TargetLoc);
			Zombie->SetActorRotation(Rot);
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	//사용한 공격 타겟 변수를 초기화한다.
	UBlackboardComponent* BBComp = Ctrl->GetBlackboardComponent();
	BBComp->SetValueAsObject(TEXT("AttackTarget"), nullptr);

	//연속 공격을 대비해서 타입 변수를 한번 바꿔준다.
	AZombieBase* Zombie = Ctrl->GetPawn<AZombieBase>();
	if (IsValid(Zombie))
	{
		Zombie->ChangeAnim(EZombieAnimType::Idle);
	}
}
