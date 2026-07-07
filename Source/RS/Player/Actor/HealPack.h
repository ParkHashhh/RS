// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealPack.generated.h"

UCLASS()
class RS_API AHealPack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHealPack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> HealPackMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")

	TObjectPtr<class UNiagaraComponent> NiagaraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<class UNiagaraSystem>		HealingNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<class USoundBase> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	FTimerHandle HealingTimerHandle;

	int32 HealingTicks = 0;
	const int32 MaxHealingTicks = 6;


	bool bExploded = false;
public:
	FVector LaunchDir = FVector::ZeroVector;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
private:
	void Explode();
		
	UFUNCTION()
	void PerformHealing();
};
