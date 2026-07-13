// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectGameMode.h"
#include "RS/Player/SelectPlayerController.h"


ASelectGameMode::ASelectGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ASelectPlayerController::StaticClass();
}
