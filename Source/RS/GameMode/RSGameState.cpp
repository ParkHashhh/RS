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

	DOREPLIFETIME(ARSGameState, ObjectiveDisplayEntries);
	DOREPLIFETIME(ARSGameState, Objective1);
	DOREPLIFETIME(ARSGameState, Objective2);
	DOREPLIFETIME(ARSGameState, Objective3);
	DOREPLIFETIME(ARSGameState, StageLevel);
	DOREPLIFETIME(ARSGameState, bBossStage);
}

void ARSGameState::SetObjective1(const FStageObjective& InObjective)
{
	if (!HasAuthority() || Objective1 == InObjective)
	{
		return;
	}

	Objective1 = InObjective;

	InnerOnRepObjective1();
}

void ARSGameState::SetObjective2(const FStageObjective& InObjective)
{
	if (!HasAuthority() || Objective2 == InObjective)
	{
		return;
	}

	Objective2 = InObjective;

	InnerOnRepObjective2();
}

void ARSGameState::SetObjective3(const FStageObjective& InObjective)
{
	if (!HasAuthority() || Objective3 == InObjective)
	{
		return;
	}

	Objective3 = InObjective;

	InnerOnRepObjective3();
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

	// if (const UGameInstance* GI = GetGameInstance())
	// {
	// 	if (auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>())
	// 	{
	// 		FName LevelName = FName(UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
	// 		if (const FStageTableInfo* StageInfo = DataSubsystem->GetStageTableInfo(LevelName))
	// 		{
	// 			for (int32 Idx = 0; Idx < StageInfo->ObjectiveRowNames.Num(); ++Idx)
	// 			{
	// 				if (StageInfo->ObjectiveRowNames.IsValidIndex(Idx))
	// 				{
	// 					const FName ObjectiveRowName = StageInfo->ObjectiveRowNames[Idx];
	// 					if (const FStageObjectiveTableInfo* ObjectiveInfo
	// 						= DataSubsystem->GetStageObjectiveTableInfo(ObjectiveRowName))
	// 					{
	// 						FStageObjective Objective;
	// 						Objective.TableRowName = ObjectiveRowName;
	// 						Objective.Current = 0.0f;
	// 						Objective.Goal = ObjectiveInfo->Goal;
	// 						// 임시
	// 						switch (Idx)
	// 						{
	// 						case 0:
	// 							Objective1 = Objective;
	// 							break;
	// 						case 1:
	// 							Objective2 = Objective;
	// 							break;
	// 						case 2:
	// 							Objective3 = Objective;
	// 							break;
	// 						default:
	// 							return;
	// 						}
	//
	// 						OnObjectiveStateChangedDelegates[Idx].Broadcast();
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// }

	bInitialized = true;
	InnerOnRepInitialized();
}

void ARSGameState::OnRep_Objective1()
{
	InnerOnRepObjective1();
}

void ARSGameState::InnerOnRepObjective1()
{
	if (!Objective1.IsValid())
	{
		return;
	}

	PrevObjective1Progress = Objective1.Current;

	OnObjectiveStateChangedDelegates[0].Broadcast();
}

void ARSGameState::OnRep_Objective2()
{
	InnerOnRepObjective2();
}

void ARSGameState::InnerOnRepObjective2()
{
	if (!Objective2.IsValid())
	{
		return;
	}

	PrevObjective2Progress = Objective2.Current;

	OnObjectiveStateChangedDelegates[1].Broadcast();
}

void ARSGameState::OnRep_Objective3()
{
	InnerOnRepObjective3();
}

void ARSGameState::InnerOnRepObjective3()
{
	if (!Objective3.IsValid())
	{
		return;
	}

	PrevObjective3Progress = Objective3.Current;

	OnObjectiveStateChangedDelegates[2].Broadcast();
}

void ARSGameState::OnRep_Initialized()
{
	InnerOnRepInitialized();
}

void ARSGameState::InnerOnRepInitialized()
{
	OnGameStateReadyDelegate.Broadcast();
}

void ARSGameState::OnRep_ObjectiveDisplayEntries()
{
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
