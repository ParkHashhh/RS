// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Base.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UGameplayAbility_Base : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGameplayAbility_Base();

protected:
	// 💡 에디터(블루프린트 클래스 디폴트)에서 볼 수 있도록 변수들을 열어줍니다.
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> mCoolDownClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	float mCoolDown;

	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	FGameplayTag mCoolDownTag;

	// 내부 연산용 컨테이너 및 플래그
	mutable FGameplayTagContainer mCoolDownTags;

	bool mAbilityActive;

public:
protected:
	const FGameplayTagContainer* GetCooldownTags() const override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;


};
