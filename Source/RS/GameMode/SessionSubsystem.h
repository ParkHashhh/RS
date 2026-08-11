// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SessionSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FRSOnCreateSessionComplete, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRSOnFindSessionsComplete, bool, const TArray<FOnlineSessionSearchResult>&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRSOnJoinSessionComplete, bool, const FString&);
DECLARE_MULTICAST_DELEGATE_OneParam(FRSOnSessionUserInviteAccepted, const FOnlineSessionSearchResult&);

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
	void FindSessions(int32 MaxSearchResults = 10000, bool bIsLanQuery = false);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);
	void JoinSession(int32 SearchResultIndex);
	const TArray<FOnlineSessionSearchResult>& GetSearchResults() const;

	FRSOnCreateSessionComplete OnCreateSessionCompleteEvent;
	FRSOnFindSessionsComplete OnFindSessionsCompleteEvent;
	FRSOnJoinSessionComplete OnJoinSessionCompleteEvent;
	FRSOnSessionUserInviteAccepted OnSessionUserInviteAcceptedEvent;

private:
	bool CreateSessionInternal();

	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsCompleted(bool bWasSuccessful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, FUniqueNetIdPtr UserId,
	                                const FOnlineSessionSearchResult& InviteResult);

	void ClearCreateSessionCompleteDelegate();
	void ClearDestroySessionCompleteDelegate();
	void ClearFindSessionsCompleteDelegate();
	void ClearJoinSessionCompleteDelegate();
	void ClearSessionUserInviteAcceptedDelegate();

	IOnlineSessionPtr GetSessionInterFace() const;

private:
	int32 PendingMaxPlayers = 4;
	bool bCreateSessionAfterDestroy = false;

	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;
	FDelegateHandle FindSessionsCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle SessionUserInviteAcceptedHandle;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnSessionUserInviteAcceptedDelegate SessionUserInviteAcceptedDelegate;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
