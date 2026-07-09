// Fill out your copyright notice in the Description page of Project Settings.


#include "StageObjectiveWidget.h"

#include "Components/ListView.h"
#include "Components/StageObjectiveEntryObject.h"
#include "RS/GameMode/RSGameState.h"

void UStageObjectiveWidget::Init()
{
	if (const UWorld* World = GetWorld())
	{
		if (auto* GS = World->GetGameState<ARSGameState>())
		{
			const FStageObjective& Objective1 = GS->GetObjective1();
			if (Objective1.ClearCondition != EClearCondition::None)
			{
				auto* NewObj = NewObject<UStageObjectiveEntryObject>(this);
				NewObj->Initialize(Objective1);
				StageObjectiveEntryObjects.Add(NewObj);
			}

			const FStageObjective& Objective2 = GS->GetObjective2();
			if (Objective2.ClearCondition != EClearCondition::None)
			{
				auto* NewObj = NewObject<UStageObjectiveEntryObject>(this);
				NewObj->Initialize(Objective2);
				StageObjectiveEntryObjects.Add(NewObj);
			}

			const FStageObjective& Objective3 = GS->GetObjective3();
			if (Objective3.ClearCondition != EClearCondition::None)
			{
				auto* NewObj = NewObject<UStageObjectiveEntryObject>(this);
				NewObj->Initialize(Objective3);
				StageObjectiveEntryObjects.Add(NewObj);
			}

			StageObjectiveEntryListView->SetListItems(StageObjectiveEntryObjects);
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
