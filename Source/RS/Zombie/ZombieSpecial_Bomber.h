// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Zombie/ZombieSpecial.h"
#include "ZombieSpecial_Bomber.generated.h"

/**
 * 
 */
UCLASS()
class RS_API AZombieSpecial_Bomber : public AZombieSpecial
{
	GENERATED_BODY()

public:
	AZombieSpecial_Bomber();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"));
	TSubclassOf<UGameplayAbility> SkillAbility;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> Particle;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraSystem> NiagaraParticle;

	//이펙트 함수를 클라에서도 호출하기 위한 리플리케이트 변수
	UPROPERTY(ReplicatedUsing = OnRep_SpawnEffectClient)
	bool bIsPrintClient = false;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	virtual void InitAbility();
	virtual void SkillStart();
	virtual void SkillEnd();
	virtual float GetHalfHeight();
	virtual void OnEndAbility();
	virtual void Death();

private:
	void Explosion();
	void SpawnEffect();

public:
	UFUNCTION()
	void OnRep_SpawnEffectClient();
};
