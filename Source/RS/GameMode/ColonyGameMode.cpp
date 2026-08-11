// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyGameMode.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/Player/ColonyPlayerController.h"

AColonyGameMode::AColonyGameMode()
{
	bUseSeamlessTravel = true;
	bStartPlayersAsSpectators = true;
}

void AColonyGameMode::OnPlayerCharacterSelected(AColonyPlayerController* PlayerController, FName TextureName, FName SkillName)
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

	RestartPlayer(PlayerController);
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

