// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplaySubsystem.h"
#include "AbilitySystemComponent.h"

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

UAbilitySystemComponent* UGameplaySubsystem::GetPlayerASC() const
{
	if (CachedCharacter.IsValid())
	{
		return CachedCharacter->GetAbilitySystemComponent();
	}
	return nullptr;
}

void UGameplaySubsystem::OnPawnChanged(APawn* NewPawn)
{
	Bind();

	BindPawn(NewPawn);

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
		CachedPS->RemoveAllOnPlayerStateReady(this);
	}

	CachedPS = PS;

	if (!PS)
	{
		bPlayerStateReady = false;

		return;
	}

	PS->ExecuteOrBindOnPlayerStateReady(this, &ThisClass::OnPlayerStateReadyCallback);
}

void UGameplaySubsystem::BindPawn(APawn* NewPawn)
{
	auto* Character = Cast<APlayerCharacterBase>(NewPawn);

	if (CachedCharacter == Character)
	{
		return;
	}

	bPawnReady = false;
	bASCReady = false;

	if (CachedCharacter.IsValid())
	{
		CachedCharacter->RemoveAllOnASCReady(this);
	}

	CachedCharacter = Character;

	if (!Character)
	{
		return;
	}

	bPawnReady = true;

	Character->ExecuteOrBindOnASCReady(this, &ThisClass::OnASCReadyCallback);
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
