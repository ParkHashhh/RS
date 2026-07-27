// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GameplayCueNotify_IceGround.generated.h"

/**
 * 
 */
UCLASS()
class RS_API AGameplayCueNotify_IceGround : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TObjectPtr<class UNiagaraSystem> IceGroundVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	FVector IceGroundScale = FVector(1.f);
public:
	virtual bool OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
};
