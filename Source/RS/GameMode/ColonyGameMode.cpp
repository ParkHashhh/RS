// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyGameMode.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/Player/ColonyPlayerController.h"
#include "RS/GameMode/RSGameInstance.h"

AColonyGameMode::AColonyGameMode()
{
	bUseSeamlessTravel = true;
	bStartPlayersAsSpectators = true;
}

void AColonyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| PostLogin: %s"), *LOG_LINE_INFO, *GetNameSafe(NewPlayer));
	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| PostLogin: %s"), *LOG_LINE_INFO, *GetNameSafe(NewPlayer));
}

void AColonyGameMode::OnPlayerCharacterSelected(AColonyPlayerController* PlayerController, FName TextureName,
	FName SkillName)
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	auto* PS = PlayerController->GetPlayerState<ARSPlayerState>();
	if (!IsValid(PS))
	{
		return;
	}

	PS->SetTextureName(TextureName);
	PS->SetSkillName(SkillName);

	if (APawn* OldPawn = PlayerController->GetPawn())
	{
		OldPawn->Destroy();
	}

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Character Selected: %s, %s"), *LOG_LINE_INFO, *TextureName.ToString(), *SkillName.ToString());

	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s| Pawn Before=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));
	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| Pawn Before=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));

	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| Controller Local=%s"), *LOG_LINE_INFO, PlayerController->IsLocalController()? TEXT("true") : TEXT("false"));

	RestartPlayer(PlayerController);

	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s| Pawn After=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));
	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| Pawn After=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));
}

APlayerController* AColonyGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
                                          const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* Result = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	if (!Result)
	{
		return nullptr;
	}

	RS_PRINT_SCREEN(5, FColor::Red, TEXT("Login called! Options: [%s]"), *Options);
	RS_PRINT_SCREEN(5, FColor::Red, TEXT("HasAuthority %d"), HasAuthority());

	FString TextureName;
	FString SkillName;

	FParse::Value(*Options, TEXT("Texture="), TextureName);
	FParse::Value(*Options, TEXT("Skill="), SkillName);
	if (ARSPlayerState* PS = Result->GetPlayerState<ARSPlayerState>())
	{
		if (!TextureName.IsEmpty())
		{
			PS->SetTextureName(FName(TextureName));
		}
		if (!SkillName.IsEmpty())
		{
			PS->SetSkillName(FName(SkillName));
		}
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				if (!TextureName.IsEmpty() || !SkillName.IsEmpty())
				{
					StorageSubsystem->SetSelectedCharacter(
						!TextureName.IsEmpty() ? FName(TextureName) : StorageSubsystem->GetSelectedTextureName(),
						!SkillName.IsEmpty() ? FName(SkillName) : StorageSubsystem->GetSelectedSkillName());
				}
				StorageSubsystem->RestoreTextureSkill(PS);
			}
		}
	}

	return Result;
}
