// Fill out your copyright notice in the Description page of Project Settings.


#include "StageObjectiveWidget.h"

#include "Components/ListView.h"
#include "Components/StageObjectiveEntryObject.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/RSGameState.h"

void UStageObjectiveWidget::Init()
{
	if (const UWorld* World = GetWorld())
	{
		if (auto* GS = World->GetGameState<ARSGameState>())
		{
			GS->OnObjectiveChangedDelegate.AddUObject(this, &ThisClass::RefreshContents);
			RefreshContents();

			// const auto* DataSubsystem = GetSubsystem<UDataSubsystem>();
			// const FStageObjective& Objective1 = GS->GetObjective1();
			// if (Objective1.IsValid())
			// {
			// 	auto* NewObj = NewObject<UStageObjectiveEntryObject>(this);
			// 	NewObj->Initialize(Objective1, DataSubsystem, 0);
			// 	StageObjectiveEntryObjects.Add(NewObj);
			// }
			//
			// const FStageObjective& Objective2 = GS->GetObjective2();
			// if (Objective2.IsValid())
			// {
			// 	auto* NewObj = NewObject<UStageObjectiveEntryObject>(this);
			// 	NewObj->Initialize(Objective2, DataSubsystem, 1);
			// 	StageObjectiveEntryObjects.Add(NewObj);
			// }
			//
			// const FStageObjective& Objective3 = GS->GetObjective3();
			// if (Objective3.IsValid())
			// {
			// 	auto* NewObj = NewObject<UStageObjectiveEntryObject>(this);
			// 	NewObj->Initialize(Objective3, DataSubsystem, 2);
			// 	StageObjectiveEntryObjects.Add(NewObj);
			// }
			//
			// StageObjectiveEntryListView->SetListItems(StageObjectiveEntryObjects);
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
