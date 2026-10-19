// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/GameFramework/GameMode/StartGameMode.h"

#include "RS/GameFramework/GameState/RSGameState.h"
#include "RS/Player/Controller/StartPlayerController.h"

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
