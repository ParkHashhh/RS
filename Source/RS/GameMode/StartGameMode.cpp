// Fill out your copyright notice in the Description page of Project Settings.


#include "StartGameMode.h"
#include "RS/Player/StartPlayerController.h"

AStartGameMode::AStartGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = AStartPlayerController::StaticClass();
}
