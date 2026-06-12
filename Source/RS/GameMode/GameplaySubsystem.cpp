// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySubsystem.h"

#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/RSPlayerController.h"
#include "RS/Player/RSPlayerState.h"

void UGameplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Bind();
}

void UGameplaySubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGameplaySubsystem::OnPawnChanged(const APawn* NewPawn)
{
	bPawnReady = NewPawn != nullptr;
	bASCReady = false;

	CheckGameplayReady();
}

void UGameplaySubsystem::Bind()
{
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
	BindPawn();
}

void UGameplaySubsystem::BindPlayerState()
{
	auto* PS = CachedPC.IsValid() ? CachedPC.Get()->GetPlayerState<ARSPlayerState>() : nullptr;
	if (!PS)
	{
		return;
	}

	CachedPS = PS;

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
	if (!Character)
	{
		return;
	}

	CachedCharacter = Character;

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
