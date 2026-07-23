// Fill out your copyright notice in the Description page of Project Settings.


#include "StageObjectiveEntryObject.h"

#include "RS/RSTypes.h"

void UStageObjectiveEntryObject::Initialize(const FStageObjective& Objective, const UDataSubsystem* DataSubsystem,
                                            int32 InIdx)
{
	if (DataSubsystem)
	{
		if (const FStageObjectiveTableInfo* Info = DataSubsystem->GetStageObjectiveTableInfo(Objective.TableRowName))
		{
			Title = Info->ObjectiveName;
			Description = Info->ObjectiveDescription;
		}
	}

	Current = Objective.Current;
	Goal = Objective.Goal;

	if (GetWorld())
	{
		if (auto* GS = GetWorld()->GetGameState<ARSGameState>())
		{
			if (GS->OnObjectiveStateChangedDelegates.IsValidIndex(InIdx))
			{
				Idx = InIdx;

				GS->OnObjectiveStateChangedDelegates[InIdx].AddUObject(this, &ThisClass::OnObjectiveStateChanged);
				OnObjectiveStateChanged();
			}
		}
	}
}

void UStageObjectiveEntryObject::OnObjectiveStateChanged()
{
	if (GetWorld())
	{
		if (const auto* GS = GetWorld()->GetGameState<ARSGameState>())
		{
			switch (Idx)
			{
			case 0:
				Current = GS->GetObjective1().Current;
				break;
			case 1:
				Current = GS->GetObjective2().Current;
				break;
			case 2:
				Current = GS->GetObjective3().Current;
				break;
			default:
				break;
			}
		}
	}
}
