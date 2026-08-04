// Fill out your copyright notice in the Description page of Project Settings.


#include "RSInGameMode.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/PlayerControllerBase.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Player/Actor/TravelPortal.h"
#include "RS/Player/ColonyPlayerController.h"


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
				StorageSubsystem->RestorePlayerStageResultStat(PC);
				if (const auto* PS = PC->GetPlayerState<ARSPlayerState>())
				{
					if (StorageSubsystem->HasPendingStageResult(PS->GetUniqueId()))
					{
						if (auto* ColonyPC = Cast<AColonyPlayerController>(NewPlayer))
						{
							ColonyPC->Client_ShowStageResultWidget();
						}
					}
				}
			}
		}
	}
}


void ARSInGameMode::CreateOrUpdatePortal(FName LevelName, const FString& PackagePath)
{
	if (IsValid(ActivePortal))
	{
		ActivePortal->Destroy();
		ActivePortal = nullptr;
	}

	if (IsValid(TravelPortalClass))
	{
		ActivePortal = GetWorld()->SpawnActor<ATravelPortal>(TravelPortalClass, PortalSpawnTransform);
		if (IsValid(ActivePortal))
		{
			ActivePortal->SetTarget(LevelName, PackagePath);
		}
	}
}
