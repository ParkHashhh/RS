// Fill out your copyright notice in the Description page of Project Settings.


#include "RSGameState.h"

#include "Net/UnrealNetwork.h"

ARSGameState::ARSGameState()
{
}

void ARSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARSGameState, StageState);
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

void ARSGameState::Initialize()
{
	if (!HasAuthority())
	{
		return;
	}

	// TODO: 초기화 로직

	bInitialized = true;
	InnerOnRepInitialized();
}

void ARSGameState::OnRep_StageState()
{
}

void ARSGameState::OnRep_Objective1()
{
	InnerOnRepObjective1();
}

void ARSGameState::InnerOnRepObjective1()
{
	if (Objective1.ClearCondition == EClearCondition::None)
	{
		return;
	}

	PrevObjective1Progress = Objective1.Current;

	OnObjective1StateChangedDelegate.Broadcast();
}

void ARSGameState::OnRep_Objective2()
{
	InnerOnRepObjective2();
}

void ARSGameState::InnerOnRepObjective2()
{
	if (Objective2.ClearCondition == EClearCondition::None)
	{
		return;
	}

	PrevObjective2Progress = Objective2.Current;

	OnObjective2StateChangedDelegate.Broadcast();
}

void ARSGameState::OnRep_Objective3()
{
	InnerOnRepObjective3();
}

void ARSGameState::InnerOnRepObjective3()
{
	if (Objective3.ClearCondition == EClearCondition::None)
	{
		return;
	}

	PrevObjective3Progress = Objective3.Current;

	OnObjective3StateChangedDelegate.Broadcast();
}

void ARSGameState::OnRep_Initialized()
{
	InnerOnRepInitialized();
}

void ARSGameState::InnerOnRepInitialized()
{
	OnGameStateReadyDelegate.Broadcast();
}
