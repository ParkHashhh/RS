// Fill out your copyright notice in the Description page of Project Settings.


#include "DataStorageSubsystem.h"

#include "GameFramework/GameStateBase.h"
#include "RS/LogCategory.h"
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

	TempInventoryStorage.Empty();

	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		for (const auto& PS : GS->PlayerArray)
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

				TempInventoryStorage.Emplace(RSPS->GetUniqueId(), TempInventory);
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
		if (TArray<FItemStorageInfo>* Stored = TempInventoryStorage.Find(UID))
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

			TempInventoryStorage.Remove(UID);
		}
	}
}

void UDataStorageSubsystem::StoreAllPlayerStageResult()
{
}

void UDataStorageSubsystem::RestorePlayerStageResult(const ARSPlayerController* NewPlayer)
{
}
