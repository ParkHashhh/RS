// Fill out your copyright notice in the Description page of Project Settings.


#include "DataStorageSubsystem.h"

#include "GameFramework/GameStateBase.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/MainGameMode.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/Player/RSPlayerController.h"
#include "RS/Player/RSPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "RS/Save/RSSaveGame.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/GameMode/RSGameState.h"

const FString UDataStorageSubsystem::SaveSlotName = TEXT("RS_SinglePlayerSave");

void UDataStorageSubsystem::StoreAllPlayerInventory()
{
	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Storing all player inventories"), *LOG_LINE_INFO);
	if (!GetWorld()->GetAuthGameMode())
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Not host"), *LOG_LINE_INFO);
		return;
	}

	// PreviousInventoryStorage.Empty();
	PrePreviousInventoryStorage = MoveTemp(PreviousInventoryStorage);

	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (const APlayerState* PS : GS->PlayerArray)
		{
			if (const auto* RSPS = Cast<ARSPlayerState>(PS))
			{
				const TArray<FInventoryEntry>& Entries = RSPS->GetInventoryComponent()->GetEntries();

				TArray<FItemStorageInfo> TempInventory;
				TempInventory.Reserve(Entries.Num());
				for (const FInventoryEntry& Entry : Entries)
				{
					if (!Entry.Item->GetItemRowHandle().RowName.IsNone() && Entry.Item->GetItemCount() > 0)
					{
						TempInventory.Emplace(Entry.Item->GetItemRowHandle().RowName, Entry.Item->GetItemCount());
					}
				}

				RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Storing player inventory. Count: %d"),
							 *LOG_LINE_INFO, TempInventory.Num());

				PreviousInventoryStorage.Emplace(RSPS->GetUniqueId(), TempInventory);
			}
		}
	}
}

void UDataStorageSubsystem::RestorePlayerInventory(const ARSPlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Restoring player inventory"), *LOG_LINE_INFO);

	if (const auto* PS = NewPlayer->GetPlayerState<ARSPlayerState>())
	{
		const FUniqueNetIdRepl& UID = PS->GetUniqueId();
		if (TArray<FItemStorageInfo>* Stored = PreviousInventoryStorage.Find(UID))
		{
			RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Restoring player inventory. Count: %d"),
			             *LOG_LINE_INFO, Stored->Num());
			if (UInventoryComponent* InvenComp = PS->GetInventoryComponent())
			{
				for (const auto& [ItemRowName, Count] : *Stored)
				{
					InvenComp->AddItem(ItemRowName, Count);
				}
			}

			// 그대로 보존 후 다음 StoreAllPlayerInventory에서 PrePrevious로 move해야 한다
			// PreviousInventoryStorage.Remove(UID);
		}
	}
}

void UDataStorageSubsystem::ClearPlayerInventory(const FUniqueNetIdRepl& UID)
{
	PreviousInventoryStorage.Remove(UID);
	PrePreviousInventoryStorage.Remove(UID);

}

void UDataStorageSubsystem::RestoreTextureSkill(ARSPlayerState* PS)
{
	if (!PS)
	{
		return;
	}

	const FUniqueNetIdRepl& UID = PS->GetUniqueId();
	if (const FName* Texture = PendingTextureRestore.Find(UID))
	{
		PS->LoadSavedNames(*Texture, PendingSkillRestore.FindRef(UID));
		PendingTextureRestore.Remove(UID);
		PendingSkillRestore.Remove(UID);
	}
}

void UDataStorageSubsystem::SavePlayerDataToDisk(const ARSPlayerState* PS)
{
	if (!PS)
	{
		return;
	}

	auto* SaveObject = Cast<URSSaveGame>(UGameplayStatics::CreateSaveGameObject(URSSaveGame::StaticClass()));
	if (!SaveObject)
	{
		return;
	}

	if (const UInventoryComponent* InvenComp = PS->GetInventoryComponent())
	{
		for (const FInventoryEntry& Entry : InvenComp->GetEntries())
		{
			if (Entry.IsValid())
			{
				SaveObject->SavedInventory.Emplace(Entry.Item->GetItemRowHandle().RowName, Entry.Item->GetItemCount());
			}
		}
	}

	SaveObject->SavedTextureName = PS->GetTextureName();
	SaveObject->SavedSkillName = PS->GetSkillName();
	if (const ARSGameState* GS = PS->GetWorld() ? PS->GetWorld()->GetGameState<ARSGameState>() : nullptr)
	{
		SaveObject->SavedDifficultyStage = GS->GetDifficultyStage();
	}
	UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, 0);
}
bool UDataStorageSubsystem::LoadPlayerDataFromDisk(ARSPlayerState* PS)
{
	CurrentDifficultyStage = 1;

	if (!PS || !UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return false;
	}

	auto* SaveObject = Cast<URSSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveObject)
	{
		return false;
	}

	const FUniqueNetIdRepl& UID = PS->GetUniqueId();
	PreviousInventoryStorage.Add(UID, SaveObject->SavedInventory);
	PendingTextureRestore.Add(UID, SaveObject->SavedTextureName);
	PendingSkillRestore.Add(UID, SaveObject->SavedSkillName);
	CurrentDifficultyStage = SaveObject->SavedDifficultyStage;
	SetSelectedCharacter(SaveObject->SavedTextureName, SaveObject->SavedSkillName);

	return true;
}

void UDataStorageSubsystem::SetSelectedCharacter(FName Texture, FName Skill)
{
	SelectedTextureName = Texture;
	SelectedSkillName = Skill;
}

FName UDataStorageSubsystem::GetSelectedTextureName() const
{
	return SelectedTextureName;
}

FName UDataStorageSubsystem::GetSelectedSkillName() const
{
	return SelectedSkillName;
}

void UDataStorageSubsystem::DeletePlayerSaveFile()
{
	UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
}

bool UDataStorageSubsystem::TryGetPrePreviousInventoryStorage(const APlayerState* InState, TArray<FItemStorageInfo>& OutStorage) const
{
	const FUniqueNetIdRepl& Key = InState->GetUniqueId();
	if (PrePreviousInventoryStorage.Contains(Key))
	{
		OutStorage = PrePreviousInventoryStorage[Key];
		return true;
	}

	return false;
}

void UDataStorageSubsystem::StoreAllPlayerStageResultStat()
{
	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s: Storing all player stage result stat"), *LOG_LINE_INFO);
	auto* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (!GameMode)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Not host"), *LOG_LINE_INFO);
		return;
	}

	PreviousStageResultStatStorage.Empty();

	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (const APlayerState* PS : GS->PlayerArray)
		{
			const FUniqueNetIdRepl& Key = PS->GetUniqueId();
			PreviousStageResultStatStorage.Emplace(Key, FStageResultStatInfo(GameMode->GetKillCountAll(Key), GameMode->GetStageElapsedTime()));
			RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s| Stored player stage result stat: KillCount=%d, PlayTime=%f"),
				*LOG_LINE_INFO, GameMode->GetKillCountAll(Key), GameMode->GetStageElapsedTime());
		}
	}
}

void UDataStorageSubsystem::RestorePlayerStageResultStat(const ARSPlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s: Restoring player stage result stat"), *LOG_LINE_INFO);

	if (auto* PS = NewPlayer->GetPlayerState<ARSPlayerState>())
	{
		const FUniqueNetIdRepl& UID = PS->GetUniqueId();
		if (const FStageResultStatInfo* Stored = PreviousStageResultStatStorage.Find(UID))
		{
			PS->SetKillCount(Stored->KillCount);
			PS->SetPlayTime(Stored->PlayTime);

			RS_LOG_DEBUG(LogRSCommon, Log, TEXT("Restored player stage result stat: KillCount=%d, PlayTime=%f"), Stored->KillCount, Stored->PlayTime);

			PreviousStageResultStatStorage.Remove(UID);
		}
	}
}

void UDataStorageSubsystem::MarkPendingStageResult(const FUniqueNetIdRepl& UID)
{
	PendingStageResultPlayers.Add(UID);
}

bool UDataStorageSubsystem::ConsumePendingStageResult(const FUniqueNetIdRepl& UID)
{
	return PendingStageResultPlayers.Remove(UID) > 0;
}

bool UDataStorageSubsystem::HasPendingStageResult(const FUniqueNetIdRepl& UID) const
{
	return PendingStageResultPlayers.Contains(UID);
}

void UDataStorageSubsystem::RestoreDifficultyStage(ARSGameState* GS)
{
	if (!GS)
	{
		return;
	}

	GS->SetDifficultyStage(CurrentDifficultyStage);
}
