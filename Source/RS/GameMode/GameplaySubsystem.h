// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplaySubsystem.generated.h"

class APlayerCharacterBase;
class ARSPlayerState;
class ARSPlayerController;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGameplayReadyDelegate, bool);

/**
 *
 */
UCLASS()
class RS_API UGameplaySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsGameplayReady, meta=(AllowPrivateAccess=true))
	bool bGameplayReady = false;

	bool bPlayerStateReady = false;
	bool bPawnReady = false;
	bool bASCReady = false;

	TWeakObjectPtr<ARSPlayerController> CachedPC;
	TWeakObjectPtr<ARSPlayerState> CachedPS;
	TWeakObjectPtr<APlayerCharacterBase> CachedCharacter;

public:
	FOnGameplayReadyDelegate OnGameplayReady;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	ARSPlayerController* GetCachedPC() const { return CachedPC.Get(); }
	ARSPlayerState* GetCachedPS() const { return CachedPS.Get(); }
	APlayerCharacterBase* GetCachedCharacter() const { return CachedCharacter.Get(); }

	class UAbilitySystemComponent* GetPlayerASC() const;

	FORCEINLINE bool IsGameplayReady() const
	{
		return bGameplayReady;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Gameplay Ready")
	bool K2_IsGameplayReady() const
	{
		return IsGameplayReady();
	}

	void OnPawnChanged(const APawn* NewPawn);

private:
	void Bind();

	void BindPlayerState();
	void BindPawn();

	void OnPlayerStateReadyCallback();
	void OnASCReadyCallback();

	void CheckGameplayReady();
};
