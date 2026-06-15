// Fill out your copyright notice in the Description page of Project Settings.


#include "RSPlayerState.h"

#include "AbilitySystemComponent.h"
#include "RS/LogCategory.h"
#include "RS/RSUtils.h"

ARSPlayerState::ARSPlayerState()
{
}

void ARSPlayerState::NotifyPlayerStateReady()
{
	if (bPlayerStateReady)
	{
		return;
	}

	bPlayerStateReady = true;

	OnPlayerStateReady.Broadcast();
}

UAbilitySystemComponent* ARSPlayerState::GetASC() const
{
	if (!IsValid(GetPawn()))
	{
		return nullptr;
	}

	return RSUtils::FindASC(GetPawn());
}

void ARSPlayerState::BeginPlay()
{
	Super::BeginPlay();

	InitializePlayerState();
}

void ARSPlayerState::InitializePlayerState()
{
	// 실제 PS 초기화

	NotifyPlayerStateReady();
}
