// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayAbility_Aim.h"

#include "AbilitySystemComponent.h"
#include "RSTags.h"
#include "RS/RSTypes.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"

UGameplayAbility_Aim::UGameplayAbility_Aim()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

bool UGameplayAbility_Aim::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayTagContainer* SourceTags,
                                              const FGameplayTagContainer* TargetTags,
                                              FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// TODO: 조준 가능 여부 확인

	return true;
}

void UGameplayAbility_Aim::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo,
                                           const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC || !CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (ActorInfo->OwnerActor.IsValid())
	{
		if (const UGameInstance* GameInstance = ActorInfo->OwnerActor->GetGameInstance())
		{
			if (const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>())
			{
				if (const FPlayerDefaultInfo* PlayerDefaultInfo = DataSubsystem->GetPlayerDefaultTableInfo())
				{
					AimShotSpreadRadius = PlayerDefaultInfo->AimShotSpreadRadius;
					BaseShotSpreadRadius = PlayerDefaultInfo->BaseShotSpreadRadius;
				}
			}
		}
	}

	if (IsLocallyControlled())
	{
		if (IsValid(AimSpreadCurve))
		{
			// 타임라인 바인딩
			FOnTimelineFloat ProgressDelegate;
			ProgressDelegate.BindUFunction(
				this, GET_FUNCTION_NAME_CHECKED(UGameplayAbility_Aim, OnAimSpreadTimelineUpdate));

			float MinTime = 0.0f;
			float MaxTime = 0.0f;
			AimSpreadCurve->GetTimeRange(MinTime, MaxTime);

			float TimelineLength = MaxTime > 0.0f ? MaxTime : 0.2f;

			AimSpreadTimeline.SetTimelineLength(TimelineLength);
			AimSpreadTimeline.AddInterpFloat(AimSpreadCurve, ProgressDelegate);
		}
	}

	// 태그 모니터링
	FGameplayTag AimTag = FGameplayTag::RequestGameplayTag(RSTags::Player::AimTagName);
	AimTagChangedDelegateHandle = ASC->RegisterGameplayTagEvent(AimTag, EGameplayTagEventType::NewOrRemoved)
	                                 .AddUObject(this, &ThisClass::OnAimTagChanged);

	// 태그 초기상태 (from start / from end)
	if (ASC->HasMatchingGameplayTag(AimTag))
	{
		// 조준상태면 (0, 0)에 고정
		ServerCurrentValue = 0.f;
		ServerTargetValue = 0.f;

		if (!IsRunningDedicatedServer())
		{
			AimSpreadTimeline.PlayFromStart();
			StartTimelineTimer();
		}
	}
	else
	{
		// 비조준상태는 (0.2, 1.0)에 고정
		ServerCurrentValue = 1.f;
		ServerTargetValue = 1.f;
		if (!IsRunningDedicatedServer())
		{
			AimSpreadTimeline.ReverseFromEnd();
			StartTimelineTimer();
		}
	}

	if (ActorInfo->OwnerActor.IsValid())
	{
		if (auto* PlayerChar = Cast<APlayerCharacterBase>(ActorInfo->OwnerActor.Get()))
		{
			PlayerChar->SetIsAiming(true);
		}
	}
}

void UGameplayAbility_Aim::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                      const FGameplayAbilityActorInfo* ActorInfo,
                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                      bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo->OwnerActor.IsValid())
	{
		if (auto* PlayerChar = Cast<APlayerCharacterBase>(ActorInfo->OwnerActor.Get()))
		{
			PlayerChar->SetIsAiming(false);
		}
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayTag AimTag = FGameplayTag::RequestGameplayTag(RSTags::Player::AimTagName);
		ASC->RegisterGameplayTagEvent(AimTag, EGameplayTagEventType::NewOrRemoved).Remove(AimTagChangedDelegateHandle);
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ServerInterpolationTimerHandle);
	}

	AimSpreadTimeline.Stop();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_Aim::OnAimTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	bool bIsAiming = NewCount != 0;

	if (GetCurrentActorInfo() && GetCurrentActorInfo()->IsNetAuthority())
	{
		ServerTargetValue = bIsAiming ? 0.0f : 1.0f;

		if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(ServerInterpolationTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(ServerInterpolationTimerHandle, this,
			                                       &ThisClass::ServerInterpolateTimeline, ServerTickInterval, true);
		}

		if (IsRunningDedicatedServer())
		{
			return;
		}
	}

	// 기존 상태에 이어서 (reverse / play)
	if (bIsAiming)
	{
		// 조준이 있으면 원점으로
		AimSpreadTimeline.Reverse();
		StartTimelineTimer();
	}
	else
	{
		// 조준이 없어지면 끝으로
		AimSpreadTimeline.Play();
		StartTimelineTimer();
	}
}

void UGameplayAbility_Aim::OnAimSpreadTimelineUpdate(float Value)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return;
	}

	float CurrentShotSpread = FMath::Lerp(AimShotSpreadRadius, BaseShotSpreadRadius, Value);
	ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetShotSpreadRadiusAttribute(), CurrentShotSpread);
}

void UGameplayAbility_Aim::StartTimelineTimer()
{
	if (!GetWorld() || IsRunningDedicatedServer())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TimelineTickTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(TimelineTickTimerHandle, this, &UGameplayAbility_Aim::TickTimeline,
	                                       ClientTickInterval, true);
}

void UGameplayAbility_Aim::TickTimeline()
{
	if (AimSpreadTimeline.IsPlaying())
	{
		AimSpreadTimeline.TickTimeline(ClientTickInterval);
	}
	else
	{
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimelineTickTimerHandle);
		}
	}
}

void UGameplayAbility_Aim::ServerInterpolateTimeline()
{
	float MinTime = 0.0f;
	float MaxTime = 0.0f;

	if (IsValid(AimSpreadCurve))
	{
		AimSpreadCurve->FloatCurve.GetTimeRange(MinTime, MaxTime);
	}

	const float TransitionTime = MaxTime > 0.0f ? MaxTime : 0.2f;
	const float InterpSpeed = 1.0f / TransitionTime;

	ServerCurrentValue = FMath::FInterpConstantTo(ServerCurrentValue, ServerTargetValue, ServerTickInterval,
	                                              InterpSpeed);

	if (auto* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		float CurrentShotSpread = FMath::Lerp(AimShotSpreadRadius, BaseShotSpreadRadius, ServerCurrentValue);
		ASC->SetNumericAttributeBase(UPlayerAttributeSet::GetShotSpreadRadiusAttribute(), CurrentShotSpread);
	}

	if (FMath::IsNearlyEqual(ServerCurrentValue, ServerTargetValue))
	{
		GetWorld()->GetTimerManager().ClearTimer(ServerInterpolationTimerHandle);
	}
}
