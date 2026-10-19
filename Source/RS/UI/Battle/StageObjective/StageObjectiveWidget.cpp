// Fill out your copyright notice in the Description page of Project Settings.


#include "StageObjectiveWidget.h"

#include "Components/ListView.h"
#include "Components/StageObjectiveEntryObject.h"
#include "RS/Core/LogCategory.h"
#include "RS/GameFramework/GameState/RSGameState.h"

void UStageObjectiveWidget::Init()
{
	if (const UWorld* World = GetWorld())
	{
		if (auto* GS = World->GetGameState<ARSGameState>())
		{
			GS->OnObjectiveChangedDelegate.AddUObject(this, &ThisClass::RefreshContents);
			RefreshContents();
		}
	}
}

void UStageObjectiveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (const UWorld* World = GetWorld())
	{
		if (auto* GS = World->GetGameState<ARSGameState>())
		{
			GS->ExecuteOrBindOnGameStateReady(this, &ThisClass::Init);
		}
	}
}

void UStageObjectiveWidget::RefreshContents()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const auto* GS = World->GetGameState<ARSGameState>();
	if (!GS)
	{
		return;
	}

	const TArray<FStageObjectiveEntry>& StageObjectiveEntries = GS->GetObjectiveEntries();

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| RefreshContents CALLED, EntryCount: %d"),
		*LOG_LINE_INFO, StageObjectiveEntries.Num());
	for (const FStageObjectiveEntry& Entry : StageObjectiveEntries)
	{
		RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s|   Row: %s, Progress: %.2f"),
			*LOG_LINE_INFO, *Entry.TableRowName.ToString(), Entry.CurreentProgress);
	}

	StageObjectiveEntryObjects.Reset(StageObjectiveEntries.Num());

	const auto* DataSubsystem = GetSubsystem<UDataSubsystem>();
	for (int32 Idx = 0; Idx < StageObjectiveEntries.Num(); ++Idx)
	{
		const FStageObjectiveEntry& Entry = StageObjectiveEntries[Idx];

		auto* NewObj = NewObject<UStageObjectiveEntryObject>(this);
		auto Objective = FStageObjective(Entry, DataSubsystem);

		NewObj->Initialize(Objective, DataSubsystem, Idx);

		StageObjectiveEntryObjects.Add(NewObj);
	}

	StageObjectiveEntryListView->SetListItems(StageObjectiveEntryObjects);
}
