// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GAS/Cue/Static/GameplayCueNotify_StaticBase.h"
#include "GameplayCueNotify_PlayerStatic.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayCueNotify_PlayerStatic : public UGameplayCueNotify_StaticBase
{
	GENERATED_BODY()

public:
	UGameplayCueNotify_PlayerStatic();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	FRotator NiagaraRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	float NiagaraRandomYawRange = 30.f;

	UPROPERTY(EditAnywhere, Category = "FX")
	float NiagaraRandomPitchRange = 30.f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	UMaterialInterface* Decal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	UMaterialInterface* Splash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TMap<FName, UMaterialInterface*> SurfaceDecalMap;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	FVector SurfaceDecalSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TMap<FName, UMaterialInterface*> SurfaceSplashMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	FVector SurfaceSplashSize = FVector::ZeroVector;

public:
	bool OnExecute_Implementation(AActor* Target,
		const FGameplayCueParameters& Parameters) const;

	bool OnActive_Implementation(AActor* Target,
		const FGameplayCueParameters& Parameters) const;

	bool WhileActive_Implementation(AActor* Target,
		const FGameplayCueParameters& Parameters) const;

	bool OnRemove_Implementation(AActor* Target,
		const FGameplayCueParameters& Parameters) const;
};
