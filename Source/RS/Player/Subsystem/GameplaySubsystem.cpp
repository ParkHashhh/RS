// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/Subsystem/GameplaySubsystem.h"
#include "AbilitySystemComponent.h"

#include "RS/Core/LogCategory.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/Controller/RSPlayerController.h"
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
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| OnPawnChanged CALLED, NewPawn: %s, Map: %s"),
		*LOG_LINE_INFO, *GetNameSafe(NewPawn), *GetNameSafe(GetWorld()));

	Bind();

	BindPawn(NewPawn);

	CheckGameplayReady();
}

void UGameplaySubsystem::Bind()
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		RS_LOG_DEBUG(LogRSSession, Error, TEXT("%s| Bind FAILED, LocalPlayer is null"), *LOG_LINE_INFO);
		return;
	}

	auto* PC = Cast<ARSPlayerController>(LocalPlayer->GetPlayerController(GetWorld()));
	if (!PC)
	{
		RS_LOG_DEBUG(LogRSSession, Error, TEXT("%s| Bind FAILED, PlayerController cast failed"), *LOG_LINE_INFO);
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

		RS_LOG_DEBUG(LogRSSession, Error, TEXT("%s| BindPlayerState FAILED, PlayerState is null"), *LOG_LINE_INFO);

		return;
	}

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| BindPlayerState, ExecuteOrBindOnPlayerStateReady for PS: %s"),
		*LOG_LINE_INFO, *GetNameSafe(PS));

	PS->ExecuteOrBindOnPlayerStateReady(this, &ThisClass::OnPlayerStateReadyCallback);
}

void UGameplaySubsystem::BindPawn(APawn* NewPawn)
{
	auto* Character = Cast<APlayerCharacterBase>(NewPawn);

	if (CachedCharacter == Character)
	{
		RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| BindPawn SKIPPED, Character unchanged: %s"),
			*LOG_LINE_INFO, *GetNameSafe(Character));
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
		RS_LOG_DEBUG(LogRSSession, Error, TEXT("%s| BindPawn FAILED, Character cast failed for Pawn: %s"),
			*LOG_LINE_INFO, *GetNameSafe(NewPawn));
		return;
	}

	bPawnReady = true;

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| BindPawn, ExecuteOrBindOnASCReady for Character: %s"),
		*LOG_LINE_INFO, *GetNameSafe(Character));

	Character->ExecuteOrBindOnASCReady(this, &ThisClass::OnASCReadyCallback);
}

void UGameplaySubsystem::OnPlayerStateReadyCallback()
{
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| OnPlayerStateReadyCallback CALLED"), *LOG_LINE_INFO);

	bPlayerStateReady = true;

	CheckGameplayReady();
}

void UGameplaySubsystem::OnASCReadyCallback()
{
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| OnASCReadyCallback CALLED"), *LOG_LINE_INFO);

	bASCReady = true;

	CheckGameplayReady();
}

void UGameplaySubsystem::CheckGameplayReady()
{
	const bool bNewGameplayReady = bPlayerStateReady && bPawnReady && bASCReady;

	RS_LOG_DEBUG(LogRSSession, Warning,
		TEXT("%s| CheckGameplayReady, PlayerStateReady: %d, PawnReady: %d, ASCReady: %d -> New: %d, Old: %d"),
		*LOG_LINE_INFO, bPlayerStateReady, bPawnReady, bASCReady, bNewGameplayReady, bGameplayReady);

	if (bGameplayReady == bNewGameplayReady)
	{
		return;
	}

	bGameplayReady = bNewGameplayReady;

	OnGameplayReady.Broadcast(bNewGameplayReady);
}
