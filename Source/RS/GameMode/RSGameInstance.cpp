// Fill out your copyright notice in the Description page of Project Settings.


#include "RSGameInstance.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Kismet/GameplayStatics.h"
#include "RS/GameMode/SessionSubsystem.h"
#include "RS/RSUtils.h"
#include "Components/AudioComponent.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/LogCategory.h"
#include "Sound/SoundBase.h"

void URSGameInstance::Init()
{
	Super::Init();

	BGMComponent = NewObject<UAudioComponent>(this);
	BGMComponent->bAutoDestroy = false;
	BGMComponent->bIsUISound = true;
	BGMComponent->bIgnoreForFlushing = true;
	BGMComponent->SetSound(BGMSound);
	BGMComponent->Play();

	if (auto* OSS = Online::GetSubsystem(GetWorld()))
	{
		RS_LOG_DEBUG(LogRSInit, Log, TEXT("%s| Subsystem: %s"), *LOG_LINE_INFO, *OSS->GetSubsystemName().ToString());
		RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Subsystem: %s"), *LOG_LINE_INFO,
		                *OSS->GetSubsystemName().ToString());
	}
	else
	{
		RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s| Failed to get online subsystem"), *LOG_LINE_INFO);
		RS_PRINT_SCREEN(30.f, FColor::Cyan, TEXT("%s| Failed to get online subsystem"), *LOG_LINE_INFO);
	}

	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionUserInviteAcceptedHandle = SessionSubsystem->OnSessionUserInviteAcceptedEvent.AddUObject(
			this, &ThisClass::OnSessionUserInviteAccepted);
	}
}

void URSGameInstance::Shutdown()
{
	ClearSessionDelegates();

	Super::Shutdown();
}

void URSGameInstance::LevelLoading(const FName& LevelName, const FString& PackagePath, const FString& Options,
                                   bool bListen, bool IsServerTravel, bool bStoreData)
{
	NextLevelName = LevelName;
	NextLevelPackagePath = PackagePath;
	NextLevelOptions = Options;
	bNextLevelIsListen = bListen;
	bIsServerTravel = IsServerTravel;

	RS_LOG_DEBUG(LogRSSession, Warning,
	             TEXT("%s| LevelLoading: LevelName=%s PackagePath=%s Options=%s"),
	             *LOG_LINE_INFO,
	             *LevelName.ToString(),
	             *PackagePath,
	             *Options);

	if (IsServerTravel)
	{
		RSUtils::ServerTravelWithDataStorage(GetWorld(), TEXT("Loading"), bStoreData, true, false);
	}
	else
	{
		RSUtils::OpenLevelWithDataStorage(GetWorld(), TEXT("Loading"), bStoreData, true, FString(TEXT("")));
	}
}

void URSGameInstance::CreateSessionAndLevelLoading(const FName& LevelName, const FString& PackagePath,
                                                   const FString& Options, bool bListen, bool IsServerTravel,
                                                   bool bStoreData, int32 MaxPlayers)
{
	ClearHostSessionDelegate();
	ClearPendingClientTravelState();

	PendingHostLevelName = LevelName;
	PendingHostLevelPackagePath = PackagePath;
	PendingHostLevelOptions = Options;
	bPendingHostLevelIsListen = bListen;
	bPendingHostIsServerTravel = IsServerTravel;
	bPendingHostStoreData = bStoreData;

	USessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (!SessionSubsystem)
	{
		OnHostSessionLevelLoadingCompleteEvent.Broadcast(false);
		return;
	}

	HostSessionCompleteHandle = SessionSubsystem->OnCreateSessionCompleteEvent.AddUObject(
		this, &ThisClass::OnCreateHostSessionCompleted);
	SessionSubsystem->CreateSession(MaxPlayers);
}

void URSGameInstance::JoinSelectedSession(const FOnlineSessionSearchResult& SearchResult, const FName& TextureName,
                                          const FName& SkillName)
{
	ClearJoinSessionDelegate();
	ClearPendingClientTravelState();

	SelectedTextureName = TextureName;
	SelectedSkillName = SkillName;
	PendingClientTravelOptions = FString::Printf(TEXT("Texture=%s Skill=%s "),
	                                             *TextureName.ToString(), *SkillName.ToString());

	USessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (!SessionSubsystem)
	{
		ClearPendingClientTravelState();
		OnJoinSessionCompleteEvent.Broadcast(false, PendingClientResolvedConnectString);
		return;
	}

	JoinSessionCompleteHandle = SessionSubsystem->OnJoinSessionCompleteEvent.AddUObject(
		this, &ThisClass::OnJoinSessionCompleted);
	SessionSubsystem->JoinSession(SearchResult);
}

void URSGameInstance::OnCreateHostSessionCompleted(bool bWasSuccessful)
{
	ClearHostSessionDelegate();

	if (!bWasSuccessful)
	{
		OnHostSessionLevelLoadingCompleteEvent.Broadcast(false);
		return;
	}

	LevelLoading(PendingHostLevelName, PendingHostLevelPackagePath, PendingHostLevelOptions,
	             bPendingHostLevelIsListen, bPendingHostIsServerTravel, bPendingHostStoreData);
	OnHostSessionLevelLoadingCompleteEvent.Broadcast(true);
}

void URSGameInstance::OnJoinSessionCompleted(bool bWasSuccessful, const FString& ConnectString)
{
	ClearJoinSessionDelegate();

	const bool bHasResolvedConnectString = bWasSuccessful && !ConnectString.IsEmpty();
	const bool bCanStartClientTravel = bHasResolvedConnectString && !PendingClientTravelOptions.IsEmpty();
	if (bCanStartClientTravel)
	{
		PendingClientResolvedConnectString = ConnectString;
		bPendingClientTravel = true;
		LevelLoading(TEXT("Colony"), TEXT("/Game/Levels/Colony"), PendingClientTravelOptions,
		             false, false, true);
	}
	else
	{
		ClearPendingClientTravelState();
	}

	OnJoinSessionCompleteEvent.Broadcast(bCanStartClientTravel, PendingClientResolvedConnectString);
}

bool URSGameInstance::ConsumePendingClientTravel(FString& OutConnectString, FString& OutOptions)
{
	if (!bPendingClientTravel || PendingClientResolvedConnectString.IsEmpty())
	{
		return false;
	}

	OutConnectString = PendingClientResolvedConnectString;
	OutOptions = PendingClientTravelOptions;
	ClearPendingClientTravelState();
	return true;
}

void URSGameInstance::OnSessionUserInviteAccepted(const FOnlineSessionSearchResult& InviteResult)
{
	ClearJoinSessionDelegate();
	ClearPendingClientTravelState();

	if (UDataStorageSubsystem* Storage = GetSubsystem<UDataStorageSubsystem>())
	{
		SelectedTextureName = Storage->GetSelectedTextureName();
		SelectedSkillName = Storage->GetSelectedSkillName();
	}

	if (!SelectedTextureName.IsNone() && !SelectedSkillName.IsNone())
	{
		PendingClientTravelOptions = FString::Printf(TEXT("Texture=%s Skill=%s "),
		                                             *SelectedTextureName.ToString(), *SelectedSkillName.ToString());
	}

	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		JoinSessionCompleteHandle = SessionSubsystem->OnJoinSessionCompleteEvent.AddUObject(
			this, &ThisClass::OnJoinSessionCompleted);
	}
}

void URSGameInstance::ClearHostSessionDelegate()
{
	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		if (HostSessionCompleteHandle.IsValid())
		{
			SessionSubsystem->OnCreateSessionCompleteEvent.Remove(HostSessionCompleteHandle);
		}
	}

	HostSessionCompleteHandle.Reset();
}

void URSGameInstance::ClearJoinSessionDelegate()
{
	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		if (JoinSessionCompleteHandle.IsValid())
		{
			SessionSubsystem->OnJoinSessionCompleteEvent.Remove(JoinSessionCompleteHandle);
		}
	}

	JoinSessionCompleteHandle.Reset();
}

void URSGameInstance::ClearSessionUserInviteAcceptedDelegate()
{
	if (USessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		if (SessionUserInviteAcceptedHandle.IsValid())
		{
			SessionSubsystem->OnSessionUserInviteAcceptedEvent.Remove(SessionUserInviteAcceptedHandle);
		}
	}

	SessionUserInviteAcceptedHandle.Reset();
}

void URSGameInstance::ClearSessionDelegates()
{
	ClearHostSessionDelegate();
	ClearJoinSessionDelegate();
	ClearSessionUserInviteAcceptedDelegate();
}

void URSGameInstance::ClearPendingClientTravelState()
{
	PendingClientResolvedConnectString.Reset();
	PendingClientTravelOptions.Reset();
	bPendingClientTravel = false;
}

USessionSubsystem* URSGameInstance::GetSessionSubsystem() const
{
	return GetSubsystem<USessionSubsystem>();
}

void URSGameInstance::StopBGM()
{
	if (IsValid(BGMComponent))
	{
		BGMComponent->Stop();
	}
}
