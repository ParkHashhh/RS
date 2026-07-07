// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainGameMode.generated.h"

/**
 *
 */
UCLASS()
class RS_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainGameMode();

	virtual void StartPlay() override;

protected:
	// TODO: 데이터로 스테이지 초기화 변경 필요
	virtual void StartStage();
};
