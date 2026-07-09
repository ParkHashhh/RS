// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
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

public:
	virtual void SkillStart();
	virtual void SkillEnd();
	
};
