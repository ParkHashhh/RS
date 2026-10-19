// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Zombie/Special/ZombieSpecial.h"
#include "ZombieSpecial_Stalker.generated.h"


UCLASS()
class RS_API AZombieSpecial_Stalker : public AZombieSpecial
{
	GENERATED_BODY()

public:
	AZombieSpecial_Stalker();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"));
	TSubclassOf<UGameplayAbility> SkillAbility;

	//초기 플레이어 스토킹 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta=(AllowPrivateAccess = "true"))
	float InitStalkingDistance;

	//스토킹 상태인지를 확인하는 변수
	UPROPERTY()
	bool bIsInStalking = false;

	//스토킹 시작 지점에 도달한 순간 좀비가 위치한 각도
	UPROPERTY()
	float FirstDegree;

	//실제 스토킹 원 궤도 생성에 사용할 반지름
	UPROPERTY()
	float RealStalkingDistance;

	//스킬(도약 공격) 사용을 위한 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> JumpCurve;
	float JumpReadyTime = 0.65f;
	float JumpTime = 0.5f;
	float JumpFinishTime = 0.45f;
	float CurrentTime = 0.f;
	float JumpMaxHeight = 150.f;
	bool bIsJumping = false;

	FVector TargetLocBeforeJump;
	FVector ZombieLocBeforeJump;

	//불투명도 조절을 위한 불리언 변수 (불투명 <-> 반투명)
	UPROPERTY(ReplicatedUsing = OnRep_ChangeOpacity)
	bool bIsOpacity = false;

	FDelegateHandle CooldownTagHandle;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual void InitStatData(const FZombieDefaultInfo* Data);
	virtual void InitAbility();
	virtual void Attack();
	virtual void SkillStart();
	virtual void SkillEnd();
	virtual float GetHalfHeight();
	virtual void PreparationSkill();

public:
	void SetStalkerOpacity(float Value);

private:
	void OnCooldownTagChange(const FGameplayTag Tag, int32 NewCount);

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_LeapAttackEnd();

	UFUNCTION()
	void OnRep_ChangeOpacity();
};
