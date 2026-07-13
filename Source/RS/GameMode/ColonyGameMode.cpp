// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyGameMode.h"

AColonyGameMode::AColonyGameMode()
{
}

APlayerController* AColonyGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole,
	const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	APlayerController* Result = Super::Login(NewPlayer, InRemoteRole, Portal,
		Options, UniqueId, ErrorMessage);
	return Result;
}
