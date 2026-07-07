// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

#include "RSGameState.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/PlayerControllerBase.h"

AMainGameMode::AMainGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMainGameMode::StartPlay()
{
	Super::StartPlay();

	StartStage();
}

void AMainGameMode::StartStage()
{
	if (auto* GS = GetGameState<ARSGameState>())
	{
		GS->Initialize();
	}
}
