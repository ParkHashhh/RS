// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Core/RSConstants.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "ZombieAIController.generated.h"

class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;

UENUM()
enum class EDetectPerceptionType
{
	None,
	Sight,
	Hearing
};

UCLASS()
class RS_API AZombieAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZombieAIController(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> BT;

	UPROPERTY()
	float StuckTime = 0.f;

	UPROPERTY()
	FVector EndLoc = FVector::ZeroVector;

	UPROPERTY()
	float PrevSpeed = 0.f;

	UPROPERTY()
	FVector PrevLocation;

	UPROPERTY()
	EDetectPerceptionType TargetPerceptionType;

	//상향 점프시 속도 중복 증가를 막기 위한 불리언 변수
	bool bIsChangeUPJumpSpeed = false;

	//종료 타이밍에 도약 시작에 들어가게 하지 않기 위한 불리언 변수 (타이밍 오류 해결)
	UPROPERTY()
	bool bIsJumpEndNow = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	//각각의 감각에 대한 설정이 필요한 경우 함수를 이곳에 추가한다.
	void SetDetectRange_Sight(float Range);
	void SetDetectRange_Hearing(float Range);

public:
	UFUNCTION()
	void OnTarget(AActor* Actor, FAIStimulus Stimulus);

public:
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

public:
	void ActivateBT();
	void SetZeroSpeed();
	void RollBackSpeed();
	void MoveStart_JS();
	void MoveStart_JE();
	bool GetIsJumpEndNow();
};
