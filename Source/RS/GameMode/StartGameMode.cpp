// Fill out your copyright notice in the Description page of Project Settings.


#include "StartGameMode.h"

#include "RSGameState.h"
#include "RS/Player/StartPlayerController.h"

AStartGameMode::AStartGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AStartPlayerController::StaticClass();
}

void AStartGameMode::Init()
{
	if (const UWorld* World = GetWorld())
	{
		if (auto* GS = World->GetGameState<ARSGameState>())
		{

		}
	}
}
