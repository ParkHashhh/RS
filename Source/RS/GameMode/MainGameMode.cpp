// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

#include "RSGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/Zombie/ZombieManager.h"

AMainGameMode::AMainGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseSeamlessTravel = true;

	StageZombieManagerComponent = CreateDefaultSubobject<UZombieManager>(TEXT("ZombieManager"));
}

void AMainGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bStartStage)
	{
		StageElapsedTime += DeltaSeconds;
	}
}

void AMainGameMode::StartPlay()
{
	Super::StartPlay();

	StartStage();
}

int32 AMainGameMode::GetKillCountAll(const FUniqueNetIdRepl& PlayerUID) const
{
	int32 KillCount = 0;

	for (const auto& KillInfo : KillInfos)
	{
		if (KillInfo.UniqueNetId == PlayerUID)
		{
			KillCount += KillInfo.KillCount;
		}
	}

	return KillCount;
}

int32 AMainGameMode::GetKillCountAll(const APlayerState* PlayerState) const
{
	return PlayerState ? GetKillCountAll(PlayerState->GetUniqueId()) : 0;
}

int32 AMainGameMode::GetKillCountAll(FName MonsterRowName) const
{
	int32 KillCount = 0;

	for (const auto& KillInfo : KillInfos)
	{
		if (KillInfo.MonsterRowName == MonsterRowName)
		{
			KillCount += KillInfo.KillCount;
		}
	}

	return KillCount;
}

int32 AMainGameMode::GetKillCount(const FUniqueNetIdRepl& PlayerUID, FName MonsterRowName) const
{
	const FKillInfo* Found = KillInfos.FindByPredicate([&PlayerUID, &MonsterRowName](const auto& KillInfo)
	{
		if (KillInfo.UniqueNetId == PlayerUID && KillInfo.MonsterRowName == MonsterRowName)
		{
			return true;
		}

		return false;
	});

	return Found ? Found->KillCount : 0;
}

int32 AMainGameMode::GetKillCount(const APlayerState* PlayerState, FName MonsterRowName) const
{
	return PlayerState ? GetKillCount(PlayerState->GetUniqueId(), MonsterRowName) : 0;
}

float AMainGameMode::GetStageElapsedTime() const
{
	return StageElapsedTime;
}

void AMainGameMode::OnDead(const APawn* InInstigator, const IKillableInterface* Killed)
{
	// 지금은 좀비 죽음만 저리
	auto* Monster = Cast<AZombieBase>(Killed);
	if (!Monster)
	{
		return;
	}

	const APlayerState* PS = InInstigator->GetPlayerState();
	if (!PS)
	{
		return;
	}

	OnMonsterDead(PS, Monster);
}

void AMainGameMode::OnMonsterDead(const APlayerState* PlayerState, const IKillableInterface* Killable)
{
	if (!PlayerState || !Killable)
	{
		return;
	}

	auto* Monster = Cast<AZombieBase>(Killable);
	if (!Monster)
	{
		return;
	}

	FName MonsterRowName = Monster->GetTableRowName();
	if (MonsterRowName.IsNone())
	{
		return;
	}

	FUniqueNetIdRepl PlayerUID = PlayerState->GetUniqueId();

	FKillInfo* Found = KillInfos.FindByPredicate([&PlayerUID, &MonsterRowName](const FKillInfo& KillInfo)
	{
		return KillInfo.UniqueNetId == PlayerUID && KillInfo.MonsterRowName == MonsterRowName;
	});

	if (Found)
	{
		Found->KillCount++;
	}
	else
	{
		Found = &KillInfos.Emplace_GetRef(FKillInfo(PlayerUID, MonsterRowName, 1));
	}

	OnKillInfoChanged.Broadcast(Found);
}

TObjectPtr<UZombieManager> AMainGameMode::GetZombieManager()
{
	return StageZombieManagerComponent;
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	OnKillInfoChanged.AddUObject(this, &ThisClass::HandleKillInfoChanged);
}

void AMainGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnKillInfoChanged.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void AMainGameMode::StartStage()
{
	bStartStage = true;

	Initialize();

	if (auto* GS = GetGameState<ARSGameState>())
	{
		GS->Initialize(StageObjectiveEntries);
	}
}

void AMainGameMode::EndStage()
{
	Clear();
}

void AMainGameMode::Clear()
{
	StageElapsedTime = 0.0f;
	KillInfos.Empty();
}

void AMainGameMode::Initialize()
{
	InitializeStageObjective();
}

void AMainGameMode::InitializeStageObjective()
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>())
		{
			const FName StageRowName(*UGameplayStatics::GetCurrentLevelName(this));
			if (const FStageTableInfo* StageInfo = DataSubsystem->GetStageTableInfo(StageRowName))
			{
				for (int32 Idx = 0; Idx < StageInfo->ObjectiveRowNames.Num(); ++Idx)
				{
					const FName ObjectiveRowName = StageInfo->ObjectiveRowNames[Idx];
					StageObjectiveEntries.Emplace(ObjectiveRowName, 0.0f, Idx);
				}
			}
		}
	}
}

const TArray<FStageObjectiveEntry>& AMainGameMode::GetStageObjectiveEntries() const
{
	return StageObjectiveEntries;
}

const FStageObjectiveEntry* AMainGameMode::GetStageObjectiveEntry(int32 Index) const
{
	if (!StageObjectiveEntries.IsValidIndex(Index))
	{
		return nullptr;
	}

	return &StageObjectiveEntries[Index];
}

void AMainGameMode::HandleKillInfoChanged(const FKillInfo* KillInfo)
{
	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	const auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>();
	if (!DataSubsystem)
	{
		return;
	}

	if (FStageObjectiveEntry* Entry = StageObjectiveEntries.FindByPredicate(
		[&KillInfo, &DataSubsystem](const FStageObjectiveEntry& Entry)
		{
			if (const FStageObjectiveTableInfo* TableInfo = DataSubsystem->GetStageObjectiveTableInfo(
				Entry.TableRowName))
			{
				if (TableInfo->ClearCondition == EClearCondition::KillEnemy)
				{
					if (TableInfo->TargetTableRowName == KillInfo->MonsterRowName
						|| TableInfo->TargetTableRowName.IsNone())
					{
						return true;
					}
				}
				else if (TableInfo->ClearCondition == EClearCondition::KillBoss)
				{
					if (TableInfo->TargetTableRowName == KillInfo->MonsterRowName)
					{
						return true;
					}
				}
			}

			return false;
		}))
	{
		if (auto* GS = GetGameState<ARSGameState>())
		{
			GS->SetProgress(Entry->TableRowName, KillInfo->KillCount);
		}
	}
}
