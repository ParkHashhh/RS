// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZombieBase.h"
#include "ZombieBoss.generated.h"

class UZombieBossAttributeSet;
class AZombieSpawnPoint;
class APlayerCharacterBase;

UCLASS()
class RS_API AZombieBoss : public AZombieBase
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AZombieBoss();

protected:
	UPROPERTY()
	TObjectPtr<UZombieBossAttributeSet> BossAttributeSet;

	//몽타주를 상체만 재생해야 할 경우 true로 바꿔준다.
	UPROPERTY(Replicated)
	bool bIsOnlyUpperPlay = false;

	//보스가 소환스킬 사용을 명령하기 위해 스폰포인트를 배열로 들고 있게 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPirvateAccess = "true"))
	TArray<TObjectPtr<AZombieSpawnPoint>> SpawnPointArray;

	//타겟을 바꾸는 인터벌 타임
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPirvateAccess = "true"))
	float UpdateTargetInterval;

	//가한 데미지를 기록하기 위한 맵
	UPROPERTY(VisibleAnywhere)
	TMap<AActor*, float> Damageboard;

	//타겟 업데이트 함수를 호출하기 위한 타이머
	FTimerHandle UpdateTargetTimerHandle;

	//보스는 필드에 미리 배치된 상태로 시작함
	//게임스테이트가 준비될 때 까지 데이터 설정을 미루기 위한 타이머 핸들
	FTimerHandle InitDataTimerHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Attack();
	virtual void RangeAttack();
	virtual void RangeAttackEnd();
	virtual void InitStatData(const FZombieDefaultInfo* Data);
	virtual void UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType);
	virtual void CallBackHP(AActor* InstigatorActor);
	//보스에서 사용할 데미지도 같이 넘겨받는 CallBackHP 함수
	virtual void CallBackHP(AActor* InstigatorActor, float Damage);
	virtual void InitAbility();
	virtual float GetHalfHeight();
	virtual void SetLandingLoc();
	virtual void ApplyLandingAttack();
	virtual void LaunchProjectile();
	virtual void SpawnStone();
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

protected:
	void ChangeTarget();

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanUseMeleeAttack();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanUseMiddleAttack();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CanUseRangeAttack();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool CheckOnlyUpperPlay();

	virtual bool CanUseMeleeAttack_Implementation();
	virtual bool CanUseMiddleAttack_Implementation();
	virtual bool CanUseRangeAttack_Implementation();
	virtual bool CheckOnlyUpperPlay_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void PlayBossSkillSound(int32 Number);

	UFUNCTION(NetMulticast, Reliable)
	void PlayBossSkillEffect(int32 Number);
};
