// Fill out your copyright notice in the Description page of Project Settings.


#include "RSInGameMode.h"
#include "RS/Player/PlayerControllerBase.h"
#include "RS/LogCategory.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/GameMode/RSGameInstance.h"

ARSInGameMode::ARSInGameMode()
{
	bUseSeamlessTravel = true;
}

UClass* ARSInGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	UE_LOG(LogTemp, Warning, TEXT("=== GetDefaultPawnClassForController CALLED ==="));
	if (ARSPlayerState* PS = InController->GetPlayerState<ARSPlayerState>())
	{
		FString TextureNameStr = PS->GetTextureName().ToString();

		if (TextureNameStr.Contains(TEXT("SurvivalMan")))
		{
			return MaleCharacterClass;
		}
		else if (TextureNameStr.Contains(TEXT("SurvivalGirl")))
		{
			return FeMaleCharacterClass;
		}
	}
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


void ARSInGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Handling starting new player"), *LOG_LINE_INFO);

	if (const auto* PC = Cast<ARSPlayerController>(NewPlayer))
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				StorageSubsystem->RestorePlayerInventory(PC);
			}
		}
	}
}
