// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GameplayCueNotify_StaticBase.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayCueNotify_StaticBase : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGameplayCueNotify_StaticBase();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<class UParticleSystem>		Particle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<class UNiagaraSystem>		Niagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<class USoundBase>			Sound;


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
