// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "../../GAS/BaseAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"


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
	//원래는 애니메이션 변경 시 공격 함수를 호출하고 이곳에서 공격 어빌리티가 발동된다.
	//테스트를 위해 바로 일단 이곳에서 어빌리티를 발동시켜 확인한다.
	Zombie->ChangeAnim(EZombieAnimType::Attack);

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

	//공격 모션이 발동되면 끝까지 재생되도록 보장해야 한다.
	bool IsAttacking = BlackboardComp->GetValueAsBool(TEXT("Attacking"));
	if (!IsAttacking)
	{
		BlackboardComp->SetValueAsBool(TEXT("Attacking"), true);
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

		UBaseAttributeSet* ZombieAttri = Zombie->GetAttributeSet();
		float AttackRange = ZombieAttri->GetAttackDistance();

		if (Distance > AttackRange)
		{
			//사거리를 벗어났다면 공격 태스크는 종료되어야 한다.
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		else
		{
			//사거리 안이라면 공격을 위해 좀비의 몸을 돌려준다.
			FRotator Rot = UKismetMathLibrary::FindLookAtRotation(ZombieLoc, TargetLoc);
			Zombie->SetActorRotation(Rot);
		}
	}
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	//사용한 공격 타겟 변수를 초기화한다.
	UBlackboardComponent* BBComp = Ctrl->GetBlackboardComponent();
	BBComp->SetValueAsObject(TEXT("AttackTarget"), nullptr);
}
