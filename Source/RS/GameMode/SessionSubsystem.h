// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SessionSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FRSOnCreateSessionComplete, bool);

/**
 *
 */
UCLASS()
class RS_API USessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void CreateSession(int32 MaxPlayers = 4);

	FRSOnCreateSessionComplete OnCreateSessionCompleteEvent;

private:
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);

	IOnlineSessionPtr GetSessionInterFace() const;

private:
	FDelegateHandle CreateSessionCompleteHandle;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
};
