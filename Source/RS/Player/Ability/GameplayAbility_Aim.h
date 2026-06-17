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

private:
	static constexpr float ServerTickRate = 60.f;
	static constexpr float ServerTickInterval = 1.f / ServerTickRate;
	static constexpr float ClientTickRate = 120.f;
	static constexpr float ClientTickInterval = 1.f / ClientTickRate;

public:
	UGameplayAbility_Aim();

private:
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UCurveFloat> AimSpreadCurve;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float AimShotSpreadRadius;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float BaseShotSpreadRadius;

	FTimeline AimSpreadTimeline;
	FTimerHandle TimelineTickTimerHandle;
	FTimerHandle ServerInterpolationTimerHandle;

	float ServerCurrentValue;
	float ServerTargetValue;

	FDelegateHandle AimTagChangedDelegateHandle;

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	void OnAimTagChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void OnAimSpreadTimelineUpdate(float Value);

	void StartTimelineTimer();
	void TickTimeline();

	void ServerInterpolateTimeline();
};
