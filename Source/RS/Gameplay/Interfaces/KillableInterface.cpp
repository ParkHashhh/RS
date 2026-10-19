// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Gameplay/Interfaces/KillableInterface.h"

#include "RS/GameFramework/GameMode/MainGameMode.h"


// Add default functionality here for any IKillableInterface functions that are not pure virtual.
void IKillableInterface::OnDead(const UWorld* World, const APawn* Instigator) const
{
	if (!World || !Instigator)
	{
		return;
	}

	auto* GM = World->GetAuthGameMode<AMainGameMode>();
	if (!GM)
	{
		return;
	}

	GM->OnDead(Instigator, this);
}
