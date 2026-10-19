// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_BossSkill4.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/Zombie/ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"
#include "RS/Core/LogCategory.h"
#include "RS/Zombie/Animation/ZombieAnimInstance.h"
#include "RS/Zombie/Boss/ZombieBoss.h"
#include "RS/Player/Character/PlayerCharacterBase.h"

UBTTask_BossSkill4::UBTTask_BossSkill4()
{
	NodeName = TEXT("Skill4");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

uint16 UBTTask_BossSkill4::GetInstanceMemorySize()	const
{
	return sizeof(FReadyToUse);
}

EBTNodeResult::Type UBTTask_BossSkill4::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
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

	//만약 타겟이 있는데 타겟이 죽은 상태라면 마찬가지로 동작하지 못하게 해야 한다.
	APlayerCharacterBase* CastedTarget = Cast<APlayerCharacterBase>(Target);
	if (!IsValid(CastedTarget) || CastedTarget->IsDead())
	{
		BlackboardComp->SetValueAsObject(TEXT("Target"), nullptr);
		return EBTNodeResult::Failed;
	}

	//해당 스킬이 쿨타임이라면 다음 태스크로 넘어간다.
	UAbilitySystemComponent* ASC = Zombie->GetAbilitySystemComponent();
	if (!ASC || ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill4Cooldown"))))
	{
		return EBTNodeResult::Failed;
	}

	AZombieBoss* BZombie = Cast<AZombieBoss>(Zombie);
	if (IsValid(BZombie))
	{
		//만약 스킬 준비 함수가 false를 반환했다면 아직 애니메이션을 재생하면 안된다.
		if (!BZombie->Skill4Prepared())
		{
			//노드메모리에서 변수를 꺼내서 준비가 안되었다는 정보를 담는다.
			FReadyToUse* Ready = (FReadyToUse*)NodeMemory;
			Ready->IsReady = false;

			//애니메이션 변경 없이 Tick으로 넘어간다 : 아직 태스크가 종료되면 안된다.
			return EBTNodeResult::InProgress;
		}
	}

	//좀비의 애니메이션을 변경한다.
	Zombie->ChangeAnim(EZombieAnimType::Boss_Skill4);
	//블랙보드의 '공격중' 변수를 true로 바꾼다.
	BlackboardComp->SetValueAsBool(TEXT("Attacking"), true);

	return EBTNodeResult::InProgress;
}

void UBTTask_BossSkill4::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
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

	//만약 타겟이 있는데 타겟이 죽은 상태라면 마찬가지로 동작하지 못하게 해야 한다.
	APlayerCharacterBase* CastedTarget = Cast<APlayerCharacterBase>(Target);
	if (!IsValid(CastedTarget) || CastedTarget->IsDead())
	{
		BlackboardComp->SetValueAsObject(TEXT("Target"), nullptr);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	//만약 노드메모리에서 준비 여부 정보를를 꺼냈는데 false라면 이곳에서 애니메이션을 켜줘야 한다.
	FReadyToUse* Ready = (FReadyToUse*)NodeMemory;
	if (!Ready->IsReady)
	{
		//준비 여부를 계속 체크한다.
		AZombieBoss* BZombie = Cast<AZombieBoss>(Zombie);
		if (IsValid(BZombie))
		{
			if (BZombie->Skill4Prepared())
			{
				//준비여부 함수가 성공을 반환하면 애니메이션을 이때 재생해준다.
				Zombie->ChangeAnim(EZombieAnimType::Boss_Skill4);
				//블랙보드의 '공격중' 변수를 true로 바꾼다.
				BlackboardComp->SetValueAsBool(TEXT("Attacking"), true);
				Ready->IsReady = true;
			}
			else
			{
				//준비가 안되었다면 될 때까지 기다려야 한다.
				return;
			}
		}
	}

	float Distance = Zombie->CalcDistanceToTarget(Target);

	UBaseAttributeSet* ZombieAttri = Zombie->GetAttributeSet();
	float AttackRange = ZombieAttri->GetAttackDistance();

	bool IsAttacking = BlackboardComp->GetValueAsBool(TEXT("Attacking"));

	if (!IsAttacking)
	{
		//공격이 끝나면 타겟 방향으로 머리를 돌리고 종료
		FVector ZombieLoc = Zombie->GetActorLocation();
		FVector TargetLoc = Target->GetActorLocation();
		ZombieLoc.Z = TargetLoc.Z;

		//RInterpTo 함수를 이용해 보간된 회전값을 구한다.
		FRotator CurrentRot = Zombie->GetActorRotation();
		FRotator TargetRot = UKismetMathLibrary::FindLookAtRotation(ZombieLoc, TargetLoc);

		FRotator Rot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, Zombie->GetTurnSpeed());

		Zombie->SetActorRotation(Rot);

		//좀비가 머리를 돌리는 동안 걷기 애니메이션을 출력한다
		//타겟과 좀비의 현재 회전값의 차이를 이용한다.
		float DegreeGap = FMath::Abs(FRotator::NormalizeAxis(TargetRot.Yaw - CurrentRot.Yaw));
		if (DegreeGap > 2.f)
		{
			Zombie->ChangeAnim(EZombieAnimType::Walk);
			Zombie->GetAnimInst()->SetIsTurn(true);
		}
		else
		{
			//회전이 끝났을 때 태스크를 종료시킨다
			Zombie->ChangeAnim(EZombieAnimType::CombatIdle);
			Zombie->GetAnimInst()->SetIsTurn(false);
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}
}

void UBTTask_BossSkill4::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	//연속 공격을 대비해서 타입 변수를 한번 바꿔준다.
	AZombieBase* Zombie = Ctrl->GetPawn<AZombieBase>();
	if (IsValid(Zombie))
	{
		Zombie->ChangeAnim(EZombieAnimType::CombatIdle);
	}
}
