// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletProjectile.generated.h"
class APlayerCharacterBase;

UCLASS()
class RS_API ABulletProjectile : public AActor
{
	GENERATED_BODY()
	friend class APlayerCharacterBase;
public:	
	// Sets default values for this actor's properties
	ABulletProjectile();
protected:
		virtual void BeginPlay() override;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* TracerEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

public:
	// 매 프레임 호출
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
