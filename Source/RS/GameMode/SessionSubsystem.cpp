// Fill out your copyright notice in the Description page of Project Settings.


#include "SessionSubsystem.h"

#include "Online.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "RS/LogCategory.h"

void USessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(
		this, &ThisClass::OnCreateSessionCompleted);
}

void USessionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USessionSubsystem::CreateSession(int32 MaxPlayers)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterFace();
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegate);

	TSharedPtr<FOnlineSessionSettings> Settings = MakeShared<FOnlineSessionSettings>();

	Settings->NumPublicConnections = MaxPlayers;
	Settings->bIsLANMatch = false;

	Settings->bShouldAdvertise = true;

	/*
	* 의미: 해당 세션이 플랫폼(예: Steam, Epic Games Store, PSN, Xbox Live 등)의 '프레젠스(Presence, 상태 정보)' 시스템을 활용하도록 설정합니다.
	*
	* 상세: 플레이어가 현재 어떤 게임을 하고 있는지, 어떤 로비에 참여해 있는지와 같은 상태 정보를 친구 목록에 표시합니다.
	* 세션 정보를 플랫폼 서버에 동기화하여 친구들이 내가 게임 중임을 알 수 있도록 합니다.
	 */
	Settings->bUsesPresence = true;

	Settings->bAllowJoinInProgress = true;

	/*
	* 의미: 친구들이 플랫폼의 프레젠스 정보(친구 목록)를 통해 해당 세션에 직접 참가(Join)하는 것을 허용합니다.
	*
	* 상세: Steam이나 Epic Games 인터페이스의 친구 목록에서 내 아이콘을 마우스 우클릭했을 때 나타나는 '게임 참가(Join Game)' 버튼을 활성화합니다.
	* 초대 코드를 받거나 별도의 세션 검색을 거치지 않고도 친구 목록을 통해 바로 멀티플레이에 들어올 수 있게 됩니다.
	 */
	Settings->bAllowJoinViaPresence = true;

	Settings->bAllowInvites = true;

	Settings->bUseLobbiesIfAvailable = true;

	const ULocalPlayer* LP = GetGameInstance()->GetFirstGamePlayer();

	SessionInterface->CreateSession(*LP->GetPreferredUniqueNetId(), NAME_GameSession, *Settings);
}

void USessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	auto SessionInterface = GetSessionInterFace();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
	}

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Create Session: %s"), *LOG_LINE_INFO, bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Create Session: %s"), *LOG_LINE_INFO, bWasSuccessful ? TEXT("Success") : TEXT("Failed"));

	OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);
}

IOnlineSessionPtr USessionSubsystem::GetSessionInterFace() const
{
	if (const IOnlineSubsystem* OSS = Online::GetSubsystem(GetWorld()))
	{
		return OSS->GetSessionInterface();
	}

	return nullptr;
}
