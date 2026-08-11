// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "RSGameInstance.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FRSOnHostSessionLevelLoadingComplete, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRSOnGameInstanceJoinSessionComplete, bool, const FString&);

class USessionSubsystem;

/**
 *
 */
UCLASS()
class RS_API URSGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	FName NextLevelName;

	UPROPERTY()
	bool bIsSinglePlayerMode = true;
public:
	UPROPERTY()
	FName SelectedTextureName;

	UPROPERTY()
	FName SelectedSkillName;

	UPROPERTY()
	FString NextLevelPackagePath;

	UPROPERTY()
	bool bNextLevelIsListen;

	UPROPERTY()
	bool bIsServerTravel;

	UPROPERTY()
	FString NextLevelOptions;

	UPROPERTY()
	FString PendingClientTravelOptions;

	UPROPERTY()
	FString PendingClientResolvedConnectString;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> BGMSound;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent;

	//로딩 레벨에서 한번만 봄버 좀비가 터지게 하기 위한 불리언 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bIsBomberNiagaraLoaded = false;


public:
public:
	void SetSinglePlayerMode(bool bInSinglePlayer) { bIsSinglePlayerMode = bInSinglePlayer; }
	bool IsSinglePlayerMode() const
	{
		return bIsSinglePlayerMode;
	}
	virtual void Init() override;
	virtual void Shutdown() override;

	void LevelLoading(const FName& LevelName, const FString& PackagePath, const FString& Options, bool bListen = false,
	                  bool IsServerTravel = false, bool
	                  bStoreData = false);
	void CreateSessionAndLevelLoading(const FName& LevelName, const FString& PackagePath, const FString& Options,
	                                  bool bListen = false, bool IsServerTravel = false, bool bStoreData = false,
	                                  int32 MaxPlayers = 4);
	void JoinSelectedSession(const FOnlineSessionSearchResult& SearchResult, const FName& TextureName, const FName& SkillName);

public:
	FRSOnHostSessionLevelLoadingComplete OnHostSessionLevelLoadingCompleteEvent;
	FRSOnGameInstanceJoinSessionComplete OnJoinSessionCompleteEvent;

	void StopBGM();
	bool IsListen() const
	{
		return bNextLevelIsListen;
	}

	bool IsServerTravel() const
	{
		return bIsServerTravel;
	}

	FName GetNextLevelName() const
	{
		return NextLevelName;
	}

	const FString& GetNextLevelOptions() const
	{
		return NextLevelOptions;
	}

	const FString& GetNextLevelPackagePath() const
	{
		return NextLevelPackagePath;
	}

	const FString& GetPendingClientTravelOptions() const
	{
		return PendingClientTravelOptions;
	}

	const FString& GetPendingClientResolvedConnectString() const
	{
		return PendingClientResolvedConnectString;
	}

	bool ConsumePendingClientTravel(FString& OutConnectString, FString& OutOptions);

private:
	void OnCreateHostSessionCompleted(bool bWasSuccessful);
	void OnJoinSessionCompleted(bool bWasSuccessful, const FString& ConnectString);
	void OnSessionUserInviteAccepted(const FOnlineSessionSearchResult& InviteResult);

	void ClearHostSessionDelegate();
	void ClearJoinSessionDelegate();
	void ClearSessionUserInviteAcceptedDelegate();
	void ClearSessionDelegates();
	void ClearPendingClientTravelState();

	USessionSubsystem* GetSessionSubsystem() const;

private:
	FName PendingHostLevelName;

	UPROPERTY()
	FString PendingHostLevelPackagePath;

	UPROPERTY()
	FString PendingHostLevelOptions;

	UPROPERTY()
	bool bPendingHostLevelIsListen = false;

	UPROPERTY()
	bool bPendingHostIsServerTravel = false;

	UPROPERTY()
	bool bPendingHostStoreData = false;

	UPROPERTY()
	bool bPendingClientTravel = false;

	FDelegateHandle HostSessionCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle SessionUserInviteAcceptedHandle;

};
