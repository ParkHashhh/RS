// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePlayAbility_PlayerBase.h"
#include "Components/TimelineComponent.h"
#include "GameplayAbility_Aim.generated.h"

/**
 * Curve가 우상향 (조준: 0, 0 | 기본: 0.2, 1)
 */
UCLASS()
class RS_API UGameplayAbility_Aim : public UGameplayAbility_PlayerBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Aim();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TSubclassOf<UGameplayEffect> AimGameplayEffectClass;

	FActiveGameplayEffectHandle AimingEffectHandle;

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	virtual void OnInputReleased(float TimeHeld);
};
