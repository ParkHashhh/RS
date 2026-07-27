// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossSkill4.generated.h"


USTRUCT()
struct FReadyToUse
{
	GENERATED_BODY()
	bool IsReady = true;
};

UCLASS()
class RS_API UBTTask_BossSkill4 : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_BossSkill4();

public:
	virtual uint16 GetInstanceMemorySize()	const;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory);
	
protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		float DeltaSeconds);
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		EBTNodeResult::Type TaskResult);
	
};
