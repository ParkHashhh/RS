// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitCustom.generated.h"

USTRUCT()
struct FWaitTimer
{
	GENERATED_BODY()
	FTimerHandle WaitTimer;
	bool Finish{false};
};

UCLASS()
class RS_API UBTTask_WaitCustom : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_WaitCustom();

public:
	//노드메모리를 사용하기 위해서는 무조건 재정의 해줘야 한다.
	virtual uint16 GetInstanceMemorySize() const;
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,
	                                        uint8* NodeMemory);

protected:
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	                      float DeltaSeconds);
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	                            EBTNodeResult::Type TaskResult);

private:
	void WaitFinish(uint8* NodeMemory);
};
