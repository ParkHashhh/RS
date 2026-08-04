// Fill out your copyright notice in the Description page of Project Settings.


#include "DataStorageSubsystem.h"

#include "GameFramework/GameStateBase.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/MainGameMode.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/Player/RSPlayerController.h"
#include "RS/Player/RSPlayerState.h"

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
