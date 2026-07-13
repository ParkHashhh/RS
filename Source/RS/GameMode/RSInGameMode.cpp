// Fill out your copyright notice in the Description page of Project Settings.


#include "RSInGameMode.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/PlayerControllerBase.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/RSGameInstance.h"

ARSInGameMode::ARSInGameMode()
{
}

APlayerController* ARSInGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole,
	const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId,
	FString& ErrorMessage)
{
	URSGameInstance* GI = GetGameInstance<URSGameInstance>();

	FString TextureNameStr;
	FString SkillNameStr;
	if (GI)
	{
		TextureNameStr = GI->SelectedTextureName.ToString();
		SkillNameStr = GI->SelectedSkillName.ToString();
	}
	else
	{
		RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("Have Not GI"));
	}
	if (TextureNameStr.Contains(TEXT("SurvivalMan")))
	{
		DefaultPawnClass = MaleCharacterClass;
	}
	else if (TextureNameStr.Contains(TEXT("SurvivalGirl")))
	{
		DefaultPawnClass = FeMaleCharacterClass;
	}
	APlayerController* Result = Super::Login(NewPlayer, InRemoteRole, Portal,
		Options, UniqueId, ErrorMessage);

	APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(Result);
	if (PlayerController && GI)
	{
		PlayerController->SetTextureName(GI->SelectedTextureName);
		PlayerController->SetSkillName(GI->SelectedSkillName);
	}

	return Result;
}
