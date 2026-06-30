// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/GAS/Cue/Static/GameplayCueNotify_StaticBase.h"
#include "GameplayCueNotify_Zombie_Step.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayCueNotify_Zombie_Step : public UGameplayCueNotify_StaticBase
{
	GENERATED_BODY()

protected:
	//EPhysicalSurface 는 Enum이여서 그냥은 UPROPERTY로 못쓴다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effect")
	TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<class USoundBase>> SoundCueMap;

public:
	bool OnExecute_Implementation(AActor* Target,
		const FGameplayCueParameters& Parameters) const;
	
};
