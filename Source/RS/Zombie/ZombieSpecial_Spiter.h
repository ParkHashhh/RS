// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Zombie/ZombieSpecial.h"
#include "ZombieSpecial_Spiter.generated.h"

class AZombieProjectileBase;

UCLASS()
class RS_API AZombieSpecial_Spiter : public AZombieSpecial
{
	GENERATED_BODY()

public:
	AZombieSpecial_Spiter();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AZombieProjectileBase> NormalProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AZombieProjectileBase> SkillProjectile;

	FTimerHandle SpitTimer;
	float SpitInterval = 0.2f;

protected:
	void SpawnSpitProjectile();

public:
	virtual void Attack();
	virtual void SkillStart();
	virtual void SkillEnd();
	void StartSpit();
	void EndSpit();
	virtual float GetHalfHeight();
	virtual void SpawnZombieAttackParticle();
	
};
