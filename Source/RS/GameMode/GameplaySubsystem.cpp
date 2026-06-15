// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySubsystem.h"

#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/RSPlayerController.h"
#include "RS/Player/RSPlayerState.h"

void UGameplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameplaySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGameplaySubsystem::OnPawnChanged(const APawn* NewPawn)
{
	if (!CachedPC.IsValid())
	{
		Bind();
	}

	BindPawn();

	CheckGameplayReady();
}

void UGameplaySubsystem::Bind()
{
	if (CachedPC.IsValid())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	auto* PC = Cast<ARSPlayerController>(LocalPlayer->GetPlayerController(GetWorld()));
	if (!PC)
	{
		return;
	}

	CachedPC = PC;

	BindPlayerState();
}

void UGameplaySubsystem::BindPlayerState()
{
	auto* PS = CachedPC.IsValid() ? CachedPC.Get()->GetPlayerState<ARSPlayerState>() : nullptr;

	if (CachedPS == PS)
	{
		return;
	}

	if (CachedPS.IsValid())
	{
		CachedPS->OnPlayerStateReady.RemoveAll(this);
	}

	CachedPS = PS;

	if (!PS)
	{
		bPlayerStateReady = false;

		return;
	}

	if (PS->IsPlayerStateReady())
	{
		OnPlayerStateReadyCallback();
	}
	else
	{
		PS->OnPlayerStateReady.AddUObject(this, &ThisClass::OnPlayerStateReadyCallback);
	}
}

void UGameplaySubsystem::BindPawn()
{
	auto* Character = Cast<APlayerCharacterBase>(CachedPC.IsValid() ? CachedPC.Get()->GetPawn() : nullptr);

	if (CachedCharacter == Character)
	{
		return;
	}

	bPawnReady = false;
	bASCReady = false;

	if (CachedCharacter.IsValid())
	{
		CachedCharacter->OnASCReady.RemoveAll(this);
	}

	CachedCharacter = Character;

	if (!Character)
	{
		return;
	}

	bPawnReady = true;

	if (Character->IsASCReady())
	{
		OnASCReadyCallback();
	}
	else
	{
		Character->OnASCReady.AddUObject(this, &ThisClass::OnASCReadyCallback);
	}
}

void UGameplaySubsystem::OnPlayerStateReadyCallback()
{
	bPlayerStateReady = true;

	CheckGameplayReady();
}

void UGameplaySubsystem::OnASCReadyCallback()
{
	bASCReady = true;

	CheckGameplayReady();
}

void UGameplaySubsystem::CheckGameplayReady()
{
	const bool bNewGameplayReady = bPlayerStateReady && bPawnReady && bASCReady;

	if (bGameplayReady == bNewGameplayReady)
	{
		return;
	}

	bGameplayReady = bNewGameplayReady;

	OnGameplayReady.Broadcast(bNewGameplayReady);
}
