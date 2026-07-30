// Fill out your copyright notice in the Description page of Project Settings.


#include "RSGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"

ARSGameState::ARSGameState()
{
}

void ARSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void ARSGameState::Initialize(const TArray<FStageObjectiveEntry>& Objectives)
{
	if (!HasAuthority())
	{
		return;
	}

	for (const FStageObjectiveEntry& Objective : Objectives)
	{
		ObjectiveList.AddEntry(Objective.TableRowName, 0.0f);
	}

	OnObjectiveChangedDelegate.Broadcast();

	bInitialized = true;
	InnerOnRepInitialized();
}

void ARSGameState::OnRep_Initialized()
{
	InnerOnRepInitialized();
}

void ARSGameState::InnerOnRepInitialized()
{
	OnGameStateReadyDelegate.Broadcast();
}

FStageObjectiveEntry* ARSGameState::AddObjective(FName TableRowName, float CurrentProgress, int32 DisplayOrder)
{
	check(HasAuthority());

	FStageObjectiveEntry& Added = ObjectiveList.AddEntry(TableRowName, CurrentProgress, DisplayOrder);

	OnObjectiveAdded(Added);

	return &Added;
}

void ARSGameState::SetProgress(FName TableRowName, float NewProgress)
{
	if (FStageObjectiveEntry* Objective = FindObjective(TableRowName))
	{
		Objective->CurreentProgress = NewProgress;

		OnObjectiveChangedDelegate.Broadcast();
	}
}

FStageObjectiveEntry* ARSGameState::FindObjective(FName TableRowName)
{
	if (const int32* Index = ObjectiveList.TableRowNameIndexMap.Find(TableRowName))
	{
		return &ObjectiveList.Entries[*Index];
	}

	return nullptr;
}

const FStageObjectiveEntry* ARSGameState::FindObjective(FName TableRowName) const
{
	if (const int32* Index = ObjectiveList.TableRowNameIndexMap.Find(TableRowName))
	{
		return &ObjectiveList.Entries[*Index];
	}

	return nullptr;
}

float ARSGameState::GetObjectiveProgress(FName TableRowName) const
{
	if (const FStageObjectiveEntry* Objective = FindObjective(TableRowName))
	{
		return Objective->CurreentProgress;
	}

	return 0.0f;
}

float ARSGameState::GetObjectiveProgressRatio(FName TableRowName) const
{
	if (const FStageObjectiveEntry* Objective = FindObjective(TableRowName))
	{
		return Objective->CurreentProgress;
	}

	return 0.0f;
}

void ARSGameState::HandleEntryAdded(const FStageObjectiveEntry& Entry)
{
	ObjectiveList.RebuildIndexMap();

	if (Entry.TableRowName.IsNone())
	{
		return;
	}

	OnObjectiveAdded(Entry);
}

void ARSGameState::HandleEntryChanged(const FStageObjectiveEntry& Entry)
{
	ObjectiveList.RebuildIndexMap();

	if (Entry.TableRowName.IsNone())
	{
		return;
	}

	OnObjectiveChanged(Entry);
}

void ARSGameState::HandleEntryRemoved(const FStageObjectiveEntry& Entry)
{
	ObjectiveList.RebuildIndexMap();

	if (Entry.TableRowName.IsNone())
	{
		return;
	}

	OnObjectiveRemoved(Entry);
}

void ARSGameState::OnObjectiveAdded(const FStageObjectiveEntry& Entry)
{
	if (HasAuthority())
	{
		// 필요한 작업
	}

	OnObjectiveChangedDelegate.Broadcast();
}

void ARSGameState::OnObjectiveChanged(const FStageObjectiveEntry& Entry)
{
	if (HasAuthority())
	{
		// 필요한 작업
	}

	OnObjectiveChangedDelegate.Broadcast();
}

void ARSGameState::OnObjectiveRemoved(const FStageObjectiveEntry& Entry)
{
	if (HasAuthority())
	{
		// 필요한 작업
	}

	OnObjectiveChangedDelegate.Broadcast();
}
