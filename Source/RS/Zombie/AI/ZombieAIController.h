// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../RSConstants.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "ZombieAIController.generated.h"

class UAISenseConfig_Sight;

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
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> BT;

protected:
	virtual void BeginPlay() override;

public:
	//각각의 감각에 대한 설정이 필요한 경우 함수를 이곳에 추가한다.
	void SetDetectRange_Sight(float Range);

public:
	UFUNCTION()
	void OnTarget(AActor* Actor, FAIStimulus Stimulus);

public:
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;

public:
	void ActivateBT();
	
};
