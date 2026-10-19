// Fill out your copyright notice in the Description page of Project Settings.


#include "StageObjectiveEntryObject.h"

#include "RS/Core/LogCategory.h"
#include "RS/Core/RSTypes.h"

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
}
