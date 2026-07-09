// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "InteractableComponent.generated.h"


class APlayerCharacterBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionStateChanged, bool, bIsInteracting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionProgressChanged, float, CurrentTime, float, MaxTime);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RS_API UInteractableComponent : public USphereComponent
{
	GENERATED_BODY()

	inline static constexpr float TickInterval = 0.05f;

public:
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionStateChanged OnInteractionStateChanged;

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionProgressChanged OnInteractionProgressChanged;

private:
	bool bIsFocused = false;

	UPROPERTY(ReplicatedUsing=OnRep_Interactor, Transient)
	TWeakObjectPtr<APlayerCharacterBase> Interactor = nullptr;

	FTimerHandle InteractionTimerHandle;
	float Duration = 0.0f;
	float ElapsedTime = 0.0f;

public:
	UInteractableComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetFocus(bool bInFocus);

	bool StartInteraction(APlayerCharacterBase* InInteractor);
	void CancelInteraction();
	void CompleteInteraction();

	bool IsOccupied() const
	{
		return Interactor.IsValid();
	}

protected:
	virtual void BeginPlay() override;

	virtual void OnRegister() override;

private:
	UFUNCTION()
	void OnRep_Interactor();

	void TickInteractionProgress();
};
