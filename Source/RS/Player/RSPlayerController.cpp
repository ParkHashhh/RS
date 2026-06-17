// Fill out your copyright notice in the Description page of Project Settings.


#include "RSPlayerController.h"

#include "PlayerCharacterBase.h"
#include "RSPlayerState.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/GameplaySubsystem.h"

ARSPlayerController::ARSPlayerController()
{
}

void ARSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	NotifyPawnChanged(GetPawn());
}

void ARSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	NotifyPawnChanged(GetPawn());
}

void ARSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ARSPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	NotifyPawnChanged(aPawn);
}

void ARSPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	NotifyPawnChanged(nullptr);
}

void ARSPlayerController::NotifyPawnChanged(APawn* NewPawn)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>();
	if (!GameplaySubsystem)
	{
		return;
	}

	GameplaySubsystem->OnPawnChanged(NewPawn);
}
