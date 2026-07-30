// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Special_PreparationSkill.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../ZombieBase.h"
#include "RS/Zombie/ZombieSpecial.h"
#include "Components/CapsuleComponent.h"
#include "../../GAS/BaseAttributeSet.h"
#include "Kismet/KismetMathLibrary.h"
#include "RS/LogCategory.h"
#include "RS/Zombie/Animation/ZombieAnimInstance.h"
#include "RS/Player/PlayerCharacterBase.h"


UBTTask_Special_PreparationSkill::UBTTask_Special_PreparationSkill()
{
	NodeName = TEXT("Skill Preparation");
	bNotifyTick = true;
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_Special_PreparationSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
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

	AZombieSpecial* Zombie = Ctrl->GetPawn<AZombieSpecial>();
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

	//블랙보드 변수를 확인해 사전 준비가 필요한 좀비인 경우에만 이곳에 들어오도록 해야 한다.
	bool bIsNeedPreparation = BlackboardComp->GetValueAsBool(TEXT("IsNeedPreparation"));
	if (!bIsNeedPreparation)
	{
		return EBTNodeResult::Failed;
	}

	//공격행동 쿨타임이 있는지 확인한다.
	UAbilitySystemComponent* ASC = Zombie->GetAbilitySystemComponent();
	if (!ASC || ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.AttackCooldown"))))
	{
		return EBTNodeResult::Failed;
	}

	//스킬 쿨타임 태그가 있는지 확인한다.
	if (!ASC || ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.SZSkillCooldown"))))
	{
		return EBTNodeResult::Failed;
	}

	//그 좀비가 가지고 있는 사전 준비 함수를 이곳에서 호출한 뒤 태스크를 대기 상태로 남긴다.
	Zombie->PreparationSkill();

	BlackboardComp->SetValueAsBool(TEXT("IsInPreparation"), true);

	return EBTNodeResult::InProgress;
}
void UBTTask_Special_PreparationSkill::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
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

	AZombieSpecial* Zombie = Ctrl->GetPawn<AZombieSpecial>();
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

	bool bInPreparation = BlackboardComp->GetValueAsBool(TEXT("IsInPreparation"));
	if (!bInPreparation)
	{
		//태스크를 실패로 종료한다 : 스킬 태스크로 넘어간다.
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
}

void UBTTask_Special_PreparationSkill::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* Ctrl = OwnerComp.GetAIOwner();
	//연속 공격을 대비해서 타입 변수를 한번 바꿔준다.
	AZombieSpecial* Zombie = Ctrl->GetPawn<AZombieSpecial>();
	if (IsValid(Zombie))
	{
		Zombie->ChangeAnim(EZombieAnimType::CombatIdle);
	}
}




