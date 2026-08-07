// Fill out your copyright notice in the Description page of Project Settings.


#include "RSGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/PlayerControllerBase.h"
#include "RS/UI/Battle/BattleHUDWidget.h"
#include "RS/UI/Battle/BossHPBarWidget.h"

ARSGameState::ARSGameState()
{
}

void ARSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARSGameState, CurrentBoss);
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

void ARSGameState::OnRep_CheckZombieBoss()
{
	ChangeBossHPBarActivation();
}

void ARSGameState::ChangeBossHPBarActivation()
{
	//플레이어 컨트롤러를 가져온다.
	APlayerControllerBase* PlayerCtrl = Cast<APlayerControllerBase>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (!PlayerCtrl)
	{
		RS_PRINT_SCREEN(5.f, FColor::Black, TEXT("No PlayerCtrl"));
		return;
	}

	//컨트롤러-BattleHUD->BossHPBar 순서로 접근한다.
	UBattleHUDWidget* HUDWidget = PlayerCtrl->GetBattleHUDWidget();
	if (!HUDWidget)
	{
		RS_PRINT_SCREEN(5.f, FColor::Black, TEXT("No HUDWidget"));
		return;
	}

	UBossHPBarWidget* BossHPBarWidget = HUDWidget->GetBossHPBarWidget();
	if(!BossHPBarWidget)
	{
		RS_PRINT_SCREEN(5.f, FColor::Black, TEXT("No BossHPWidget"));
		return;
	}

	//포인터 유효성을 체크해 보스체력바 활성화/비활성화를 결정한다.
	if(CurrentBoss != nullptr)
	{
		BossHPBarWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		BossHPBarWidget->SetVisibility(ESlateVisibility::Hidden);
	}
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

void ARSGameState::RegistBoss(AZombieBoss* Boss)
{
	if (!HasAuthority() || !Boss)
	{
		return;
	}

	CurrentBoss = Boss;

	ChangeBossHPBarActivation();
}

void ARSGameState::UnRegistBoss(AZombieBoss* Boss)
{
	if (!HasAuthority() || !Boss)
	{
		return;
	}

	if (Boss == CurrentBoss)
	{
		CurrentBoss = nullptr;
	}

	ChangeBossHPBarActivation();
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
