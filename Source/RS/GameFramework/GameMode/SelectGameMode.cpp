// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/GameFramework/GameMode/SelectGameMode.h"
#include "RS/Player/Controller/SelectPlayerController.h"


ASelectGameMode::ASelectGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ASelectPlayerController::StaticClass();
}
