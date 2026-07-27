// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBoss.h"
#include "RS/RSTypes.h"
#include "ZombieBoss_Behemoth.generated.h"

class AZombieProjectileBase;
class AZombieBoss_Behemoth_PickupPoint;


UCLASS()
class RS_API AZombieBoss_Behemoth : public AZombieBoss
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AZombieBoss_Behemoth();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"));
	TArray<TSubclassOf<UGameplayAbility>> BZSkillAbilityArray;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AZombieProjectileBase> GroundSlamProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AZombieProjectileBase> ThrowingStonProjectile;

	//그라운드 슬램 투사체 발사 형태 정의
	float GSDegree = 120.f;
	int32 GSProjectileNumber = 10;

	//리프 어택 점프 형태 정의
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> JumpCurve;
	float JumpReadyTime = 0.8f;
	float JumpTime = 0.7f;
	float JumpFinishTime = 1.2f;
	float CurrentTime = 0.f;
	float JumpMaxHeight = 200.f;
	bool bIsJumping = false;
	float LandingOffset = 100.f;

	FVector TargetLoc;
	FVector MyLoc;

	//픽업 좌표를 가지고 있는 액터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AZombieBoss_Behemoth_PickupPoint> PickupPointActor;

	//픽업 좌표를 담을 배열
	UPROPERTY(VisibleAnywhere)
	TArray<FVector>		PickupPointArray;

	//스킬 발동을 위한 선 이동 작업을 하는지를 확인하는 변수
	UPROPERTY()
	bool bSkill4MoveStart = false;

	//가장 가까운 좌표를 담을 변수
	UPROPERTY()
	FVector FinishLoc;

	//현재 돌을 들고 있는지 확인하는 변수
	bool bIsPickupStone = false;

	//돌을 들고 있을 때 이동 명령이 한번만 들어가게 하기 위한 변수
	bool bIsMoveStartPickupStone = false;

	//돌을 던지기 전 머리가 다 돌아갔는지 확인하는 변수
	bool bIsFinishRotate = false;

	//돌을 뽑아서 들고 있는 과정 중인지 확인하는 변수
	bool bIsRaiseStone = false;

	//스폰된 돌을 보관할 멤버 변수
	UPROPERTY()
	TObjectPtr<AZombieProjectileBase> SpawnedStone;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual void InitAbility();
	virtual float GetHalfHeight();
	virtual void UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType);
	virtual void Attack();
	virtual void RangeAttack();
	virtual void RangeAttackEnd();
	virtual void Skill1();
	virtual void Skill1End();
	virtual void Skill2();
	virtual void Skill2End();
	virtual void Skill3();
	virtual void Skill3End();
	virtual void Skill4();
	virtual void Skill4End();
	virtual void Skill5();
	virtual void Skill5End();
	virtual void Skill6();
	virtual void Skill6End();

	virtual bool Skill4Prepared();

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_LeapAttackEnd();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartThrowing();

	virtual void SetLandingLoc();
	virtual void ApplyLandingAttack();
	virtual void LaunchProjectile();
	virtual void SpawnStone();

	virtual bool CanUseMeleeAttack_Implementation();
	virtual bool CanUseMiddleAttack_Implementation();
	virtual bool CanUseRangeAttack_Implementation();
	virtual bool CheckOnlyUpperPlay_Implementation();
};
