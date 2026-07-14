// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyGameMode.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/LogCategory.h"


AColonyGameMode::AColonyGameMode()
{
	bUseSeamlessTravel = true;
}

APlayerController* AColonyGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* Result = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

	RS_PRINT_SCREEN(5, FColor::Red, TEXT("Login called! Options: [%s]"), *Options);
	RS_PRINT_SCREEN(5, FColor::Red, TEXT("HasAuthory %d"), HasAuthority());


	FString TextureName;
	FString SkillName;

	FParse::Value(*Options, TEXT("Texture="), TextureName);
	FParse::Value(*Options, TEXT("Skill="), SkillName);

	if (ARSPlayerState* PS = Result->GetPlayerState<ARSPlayerState>())
	{
		PS->SetTextureName(FName(TextureName));
		PS->SetSkillName(FName(SkillName));
	}

	return Result;
}
