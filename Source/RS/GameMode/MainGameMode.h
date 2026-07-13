// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RSInGameMode.h"
#include "MainGameMode.generated.h"

/**
 *
 */
UCLASS()
class RS_API AMainGameMode : public ARSInGameMode
{
	GENERATED_BODY()

public:
	AMainGameMode();

	virtual void StartPlay() override;

protected:
	// TODO: 데이터로 스테이지 초기화 변경 필요
	virtual void StartStage();

public:
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole,
		const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
};
