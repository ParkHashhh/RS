// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Zombie/ZombieBase.h"
#include "ZombieSpecial.generated.h"

class UZombieStandardAttributeSet;

UCLASS()
class RS_API AZombieSpecial : public AZombieBase
{
	GENERATED_BODY()

public:
	AZombieSpecial();

protected:
	UPROPERTY()
	TObjectPtr<UZombieStandardAttributeSet> SpecialAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	bool bEnableNormalAttack;

	//스킬 사용 전 사전 준비가 필요한 경우 이 변수를 켠다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	bool bNeedPreparationToSkill = false;

	FTimerHandle InitDataTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	float AdditionalScale_Attack = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	float AdditionalScale_Defense = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	float AdditionalScale_MaxHP = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	float AdditionalScale_SkillDamage = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	float AdditionalScale_SkillDotDamage = 0;

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController);

public:
	virtual void Attack();
	virtual void InitStatData(const FZombieDefaultInfo* Data);
	virtual void UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType);
	virtual void InitAbility();
	virtual float GetHalfHeight();
	virtual void SpawnZombieAttackParticle();
	virtual void SkillStart();
	virtual void SkillEnd();
	virtual void PreparationSkill();
	virtual void OnEndAbility();
};
