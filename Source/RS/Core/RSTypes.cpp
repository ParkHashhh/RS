#include "RS/Core/RSTypes.h"

#include "Engine/GameInstance.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/GameFramework/GameState/RSGameState.h"

namespace
{
	int32 GetPlayerCountFromDataSubsystem(const UDataSubsystem* DataSubsystem)
	{
		if (const UGameInstance* GameInstance = DataSubsystem ? DataSubsystem->GetGameInstance() : nullptr)
		{
			if (const ARSGameState* GS = GameInstance->GetWorld() ? GameInstance->GetWorld()->GetGameState<ARSGameState>() : nullptr)
			{
				return FMath::Max(GS->PlayerArray.Num(), 1);
			}
		}
		return 1;
	}
}

int32 FStageObjective::GetGoal(FName RowName, const UDataSubsystem* DataSubsystem)
{
	if (RowName.IsNone() || !DataSubsystem)
	{
		return 0;
	}

	if (const FStageObjectiveTableInfo* Info = DataSubsystem->GetStageObjectiveTableInfo(RowName))
	{
		return Info->GetScaledGoal(GetPlayerCountFromDataSubsystem(DataSubsystem));
	}

	return 0;
}

FStageObjective::FStageObjective(const FStageObjectiveEntry& Entry, const UDataSubsystem* DataSubsystem)
{
	TableRowName = Entry.TableRowName;
	Current = Entry.CurreentProgress;

	if (DataSubsystem)
	{
		if (const FStageObjectiveTableInfo* Info = DataSubsystem->GetStageObjectiveTableInfo(Entry.TableRowName))
		{
			Goal = Info->GetScaledGoal(GetPlayerCountFromDataSubsystem(DataSubsystem));

			FDataTableRowHandle RowHandle;
			RowHandle.DataTable = DataSubsystem->GetStageObjectiveInfoTable();
			RowHandle.RowName = Info->TargetTableRowName;
			Targets.Emplace(RowHandle);
		}
	}
}

void FStageObjectiveEntry::PreReplicatedRemove(const FStageObjectiveList& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->HandleEntryRemoved(*this);
	}
}

void FStageObjectiveEntry::PostReplicatedAdd(const FStageObjectiveList& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->HandleEntryAdded(*this);
	}
}

void FStageObjectiveEntry::PostReplicatedChange(const FStageObjectiveList& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->HandleEntryChanged(*this);
	}
}

void FStageObjectiveList::MarkDirty(FName TableRowName)
{
	if (TableRowName.IsNone())
	{
		return;
	}

	if (FStageObjectiveEntry* Entry = FindEntry(TableRowName))
	{
		MarkItemDirty(*Entry);
	}
}

FStageObjectiveEntry* FStageObjectiveList::FindEntry(FName TableRowName)
{
	const int32* Index = TableRowNameIndexMap.Find(TableRowName);
	if (!Index)
	{
		return nullptr;
	}

	return &Entries[*Index];
}

const FStageObjectiveEntry* FStageObjectiveList::FindEntry(FName TableRowName) const
{
	const int32* Index = TableRowNameIndexMap.Find(TableRowName);
	if (!Index)
	{
		return nullptr;
	}

	return &Entries[*Index];
}

FStageObjectiveEntry& FStageObjectiveList::AddEntry(FName TableRowName, float CurrentProgress, int32 DisplayOrder)
{
	FStageObjectiveEntry& Entry = Entries.Emplace_GetRef(TableRowName, CurrentProgress, DisplayOrder);

	RebuildIndexMap();

	MarkItemDirty(Entry);

	return Entry;
}

bool FStageObjectiveList::RemoveEntry(FName TableRowName)
{
	const int32* Index = TableRowNameIndexMap.Find(TableRowName);
	if (!Index)
	{
		return false;
	}

	Entries.RemoveAt(*Index);

	RebuildIndexMap();

	MarkArrayDirty();

	return true;
}

void FStageObjectiveList::RebuildIndexMap()
{
	TableRowNameIndexMap.Empty(Entries.Num());

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (Entries[Index].TableRowName.IsNone())
		{
			continue;
		}

		TableRowNameIndexMap.Emplace(Entries[Index].TableRowName, Index);
	}
}
