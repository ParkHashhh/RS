// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ZombieProjectileBase.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UParticleSystem;
class UNiagaraSystem;

UCLASS()
class RS_API AZombieProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZombieProjectileBase();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY()
	TObjectPtr<AController> OwnerController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> Particle;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNiagaraSystem> NiagaraParticle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void SetOwnerController(AController* Ctrl);
	void SetAbilityTag(FGameplayTag Tag);
	
protected:
	UFUNCTION()
	void ProjectileStop(const FHitResult& ImpactResult);

	void SpawnGroundHitParticle(const FHitResult& ImpactResult);
};
