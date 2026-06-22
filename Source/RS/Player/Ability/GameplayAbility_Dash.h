// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility_PlayerBase.h"
#include "GameplayAbility_Dash.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayAbility_Dash : public UGameplayAbility_PlayerBase
{
	GENERATED_BODY()

public:
	UGameplayAbility_Dash();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TSubclassOf<UGameplayEffect> DashGameplayEffectClass;

	FActiveGameplayEffectHandle DashEffectHandle;
protected:
	// 어빌리티가 BeginPlay 등에서 최초 실행될 때 발동하는 함수
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
};
