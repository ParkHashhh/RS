// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZombieAIController.generated.h"

//임시로 사용할 팀 아이디 : 나중에 상위 클래스에 선언되면 제거할 예정이다.
#define	TeamNeutral	2
#define	TeamZombie	1
#define	TeamPlayer	0

UCLASS()
class RS_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZombieAIController();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<UBehaviorTree> BT;

public:
	void SetBT(UBehaviorTree* BehaviorTree);

public:
	//각각의 감각에 대한 설정이 필요한 경우 함수를 이곳에 추가한다.
	void SetDetectRange_Sight(float Range);

public:
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;
	
};
