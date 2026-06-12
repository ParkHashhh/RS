// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RS/Utils/ASCProvider.h"
#include "RSPlayerState.generated.h"

class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE(FOnPlayerStateReadyDelegate);

/**
 *
 */
UCLASS()
class RS_API ARSPlayerState : public APlayerState, public IASCProvider
{
private:
	GENERATED_BODY()

public:
	ARSPlayerState();

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsPlayerStateReady, meta=(AllowPrivateAccess=true))
	bool bPlayerStateReady = false;

public:
	FOnPlayerStateReadyDelegate OnPlayerStateReady;

	FORCEINLINE bool IsPlayerStateReady() const
	{
		return bPlayerStateReady;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Player State Ready")
	bool K2_IsPlayerStateReady() const
	{
		return IsPlayerStateReady();
	}

	void NotifyPlayerStateReady();

	virtual UAbilitySystemComponent* GetASC() const override;

protected:
	virtual void BeginPlay() override;

private:
	void InitializePlayerState();
};
