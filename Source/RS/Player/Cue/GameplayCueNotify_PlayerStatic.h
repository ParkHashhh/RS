// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GAS/Cue/Static/GameplayCueNotify_StaticBase.h"
#include "GameplayCueNotify_PlayerStatic.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FSurfaceEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	TObjectPtr<UMaterialInterface> DecalMaterial{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	TObjectPtr<UMaterialInterface> SplatterMaterial{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	TObjectPtr<UNiagaraSystem> SplatterNiagara{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> Sound{};

};

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

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TMap<TObjectPtr<UPhysicalMaterial>, FSurfaceEffectData> AttackEffectMap;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector SurfaceDecalSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector SurfaceSplashSize = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FVector SurfaceNiagaraSize = FVector::ZeroVector;
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
