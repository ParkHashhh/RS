// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionSubsystem.h"

#include "Online.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "RS/LogCategory.h"

void USessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
		this, &ThisClass::OnCreateSessionCompleted);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(
		this, &ThisClass::OnDestroySessionCompleted);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(
		this, &ThisClass::OnFindSessionsCompleted);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(
		this, &ThisClass::OnJoinSessionCompleted);
	SessionUserInviteAcceptedDelegate = FOnSessionUserInviteAcceptedDelegate::CreateUObject(
		this, &ThisClass::OnSessionUserInviteAccepted);

	if (IOnlineSessionPtr SessionInterface = GetSessionInterFace())
	{
		SessionUserInviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			SessionUserInviteAcceptedDelegate);
	}
}

void USessionSubsystem::Deinitialize()
{
	ClearCreateSessionCompleteDelegate();
	ClearDestroySessionCompleteDelegate();
	ClearFindSessionsCompleteDelegate();
	ClearJoinSessionCompleteDelegate();
	ClearSessionUserInviteAcceptedDelegate();

	Super::Deinitialize();
}

void USessionSubsystem::CreateSession(int32 MaxPlayers)
{
	ClearCreateSessionCompleteDelegate();
	ClearDestroySessionCompleteDelegate();

	PendingMaxPlayers = MaxPlayers;

	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		PendingDestroyAction = ESessionPendingAction::Create;
		DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			DestroySessionCompleteDelegate);

		if (!SessionInterface->DestroySession(NAME_GameSession))
		{
			PendingDestroyAction = ESessionPendingAction::None;
			ClearDestroySessionCompleteDelegate();
			OnCreateSessionCompleteEvent.Broadcast(false);
		}

		return;
	}

	if (!CreateSessionInternal())
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::FindSessions(int32 MaxSearchResults, bool bIsLanQuery)
{
	ClearFindSessionsCompleteDelegate();

	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (!SessionInterface.IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(false, GetSearchResults());
		return;
	}

	const ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer();
	if (!LP || !LP->GetPreferredUniqueNetId().IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(false, GetSearchResults());
		return;
	}

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = bIsLanQuery;
	SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	FindSessionsCompleteHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FindSessionsCompleteDelegate);

	if (!SessionInterface->FindSessions(*LP->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		ClearFindSessionsCompleteDelegate();
		OnFindSessionsCompleteEvent.Broadcast(false, GetSearchResults());
	}
}

void USessionSubsystem::JoinSession(const FOnlineSessionSearchResult& SearchResult)
{
	ClearJoinSessionCompleteDelegate();

	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (!SessionInterface.IsValid())
	{
		OnJoinSessionCompleteEvent.Broadcast(false, FString());
		return;
	}

	const ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer();
	if (!LP || !LP->GetPreferredUniqueNetId().IsValid() || !SearchResult.IsValid())
	{
		OnJoinSessionCompleteEvent.Broadcast(false, FString());
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		ClearDestroySessionCompleteDelegate();

		PendingDestroyAction = ESessionPendingAction::Join;
		PendingJoinSearchResult = SearchResult;

		DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			DestroySessionCompleteDelegate);

		if (!SessionInterface->DestroySession(NAME_GameSession))
		{
			PendingDestroyAction = ESessionPendingAction::None;
			ClearDestroySessionCompleteDelegate();
			OnJoinSessionCompleteEvent.Broadcast(false, FString());
		}

		return;
	}

	JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		JoinSessionCompleteDelegate);

	if (!SessionInterface->JoinSession(*LP->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult))
	{
		ClearJoinSessionCompleteDelegate();
		OnJoinSessionCompleteEvent.Broadcast(false, FString());
	}
}

void USessionSubsystem::JoinSession(int32 SearchResultIndex)
{
	if (!SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
	{
		OnJoinSessionCompleteEvent.Broadcast(false, FString());
		return;
	}

	JoinSession(SessionSearch->SearchResults[SearchResultIndex]);
}

const TArray<FOnlineSessionSearchResult>& USessionSubsystem::GetSearchResults() const
{
	static const TArray<FOnlineSessionSearchResult> EmptySearchResults;

	if (SessionSearch.IsValid())
	{
		return SessionSearch->SearchResults;
	}

	return EmptySearchResults;
}

bool USessionSubsystem::CreateSessionInternal()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	const ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer();
	if (!LP || !LP->GetPreferredUniqueNetId().IsValid())
	{
		return false;
	}

	CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShared<FOnlineSessionSettings>();

	LastSessionSettings->NumPublicConnections = PendingMaxPlayers;
	LastSessionSettings->bIsLANMatch = false;

	LastSessionSettings->bShouldAdvertise = true;

	/*
	* 의미: 해당 세션이 플랫폼(예: Steam, Epic Games Store, PSN, Xbox Live 등)의 '프레젠스(Presence, 상태 정보)' 시스템을 활용하도록 설정합니다.
	*
	* 상세: 플레이어가 현재 어떤 게임을 하고 있는지, 어떤 로비에 참여해 있는지와 같은 상태 정보를 친구 목록에 표시합니다.
	* 세션 정보를 플랫폼 서버에 동기화하여 친구들이 내가 게임 중임을 알 수 있도록 합니다.
	 */
	LastSessionSettings->bUsesPresence = true;

	LastSessionSettings->bAllowJoinInProgress = true;

	/*
	* 의미: 친구들이 플랫폼의 프레젠스 정보(친구 목록)를 통해 해당 세션에 직접 참가(Join)하는 것을 허용합니다.
	*
	* 상세: Steam이나 Epic Games 인터페이스의 친구 목록에서 내 아이콘을 마우스 우클릭했을 때 나타나는 '게임 참가(Join Game)' 버튼을 활성화합니다.
	* 초대 코드를 받거나 별도의 세션 검색을 거치지 않고도 친구 목록을 통해 바로 멀티플레이에 들어올 수 있게 됩니다.
	 */
	LastSessionSettings->bAllowJoinViaPresence = true;

	LastSessionSettings->bAllowInvites = true;

	LastSessionSettings->bUseLobbiesIfAvailable = true;

	if (!SessionInterface->CreateSession(*LP->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		ClearCreateSessionCompleteDelegate();
		return false;
	}

	return true;
}

void USessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	ClearCreateSessionCompleteDelegate();

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Create Session: %s"), *LOG_LINE_INFO,
	             bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Create Session: %s"), *LOG_LINE_INFO,
	                bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);
}

void USessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
	ClearDestroySessionCompleteDelegate();

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Destroy Session: %s"), *LOG_LINE_INFO, bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Destroy Session: %s"), *LOG_LINE_INFO, bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	const ESessionPendingAction Action = PendingDestroyAction;
	PendingDestroyAction = ESessionPendingAction::None;

	if (Action == ESessionPendingAction::Join)
	{
		if (!bWasSuccessful)
		{
			OnJoinSessionCompleteEvent.Broadcast(false, FString());
			return;
		}

		JoinSession(PendingJoinSearchResult);
		return;
	}

	if (Action != ESessionPendingAction::Create)
	{
		return;
	}

	if (!bWasSuccessful || !CreateSessionInternal())
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
	}
}

void USessionSubsystem::OnFindSessionsCompleted(bool bWasSuccessful)
{
	ClearFindSessionsCompleteDelegate();

	const int32 SearchResultCount = SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : 0;
	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Find Sessions: %s, Count: %d"), *LOG_LINE_INFO, bWasSuccessful ? TEXT("Success") : TEXT("Failed"), SearchResultCount);
	RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Find Sessions: %s, Count: %d"), *LOG_LINE_INFO, bWasSuccessful ? TEXT("Success") : TEXT("Failed"), SearchResultCount);

	OnFindSessionsCompleteEvent.Broadcast(bWasSuccessful, GetSearchResults());
}

void USessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	ClearJoinSessionCompleteDelegate();

	FString ConnectString;
	const bool bWasSuccessful = Result == EOnJoinSessionCompleteResult::Success;

	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (bWasSuccessful && SessionInterface.IsValid())
	{
		SessionInterface->GetResolvedConnectString(SessionName, ConnectString);
	}

	const bool bHasConnectString = !ConnectString.IsEmpty();
	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Join Session: %s, ConnectString: %s"), *LOG_LINE_INFO, bWasSuccessful && bHasConnectString ? TEXT("Success") : TEXT("Failed"), *ConnectString);
	RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Join Session: %s"), *LOG_LINE_INFO, bWasSuccessful && bHasConnectString ? TEXT("Success") : TEXT("Failed"));

	OnJoinSessionCompleteEvent.Broadcast(bWasSuccessful && bHasConnectString, ConnectString);
}

void USessionSubsystem::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, FUniqueNetIdPtr UserId,
                                                   const FOnlineSessionSearchResult& InviteResult)
{
	const bool bCanJoinInvite = bWasSuccessful && UserId.IsValid() && InviteResult.IsValid();

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| Session Invite Accepted Diagnostic:\n"
		"bWasSuccessful=%s\n"
		"ControllerId=%d\n"
		"UserIdValid=%s\n"
		"InviteResultValid=%s"),
		*LOG_LINE_INFO,
		bWasSuccessful ? TEXT("true") : TEXT("false"),
		ControllerId,
		UserId.IsValid() ? TEXT("true") : TEXT("false"),
		InviteResult.IsValid() ? TEXT("true") : TEXT("false"));

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Session Invite Accepted: %s"), *LOG_LINE_INFO,
	             bCanJoinInvite ? TEXT("Success") : TEXT("Failed"));
	RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Session Invite Accepted: %s"), *LOG_LINE_INFO,
	                bCanJoinInvite ? TEXT("Success") : TEXT("Failed"));

	if (!bCanJoinInvite)
	{
		OnJoinSessionCompleteEvent.Broadcast(false, FString());
		return;
	}

	OnSessionUserInviteAcceptedEvent.Broadcast(InviteResult);
	JoinSession(InviteResult);
}

void USessionSubsystem::ClearCreateSessionCompleteDelegate()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (SessionInterface.IsValid() && CreateSessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
	}

	CreateSessionCompleteHandle.Reset();
}

void USessionSubsystem::ClearDestroySessionCompleteDelegate()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (SessionInterface.IsValid() && DestroySessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
	}

	DestroySessionCompleteHandle.Reset();
}

void USessionSubsystem::ClearFindSessionsCompleteDelegate()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (SessionInterface.IsValid() && FindSessionsCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
	}

	FindSessionsCompleteHandle.Reset();
}

void USessionSubsystem::ClearJoinSessionCompleteDelegate()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (SessionInterface.IsValid() && JoinSessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
	}

	JoinSessionCompleteHandle.Reset();
}

void USessionSubsystem::ClearSessionUserInviteAcceptedDelegate()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (SessionInterface.IsValid() && SessionUserInviteAcceptedHandle.IsValid())
	{
		SessionInterface->ClearOnSessionUserInviteAcceptedDelegate_Handle(SessionUserInviteAcceptedHandle);
	}

	SessionUserInviteAcceptedHandle.Reset();
}

IOnlineSessionPtr USessionSubsystem::GetSessionInterFace() const
{
	if (const IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld()))
	{
		return OSS->GetSessionInterface();
	}

	return nullptr;
}
