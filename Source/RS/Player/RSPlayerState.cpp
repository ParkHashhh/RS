// Fill out your copyright notice in the Description page of Project Settings.


#include "RSPlayerState.h"

#include "AbilitySystemComponent.h"
#include "RS/LogCategory.h"
#include "RS/RSUtils.h"

ARSPlayerState::ARSPlayerState()
{
}

UAbilitySystemComponent* ARSPlayerState::GetASC() const
{
	if (!IsValid(GetPawn()))
	{
		RS_LOG_DEBUG(LogRSGAS, Error, TEXT("%s: Player state has no valid pawn, returning nullptr"), *LOG_LINE_INFO);
		return nullptr;
	}

	return RSUtils::FindASC(GetPawn());
}
