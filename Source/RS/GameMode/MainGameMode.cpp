// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"

#include "RSGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/Actor/TravelPortal.h"
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
		OnTimeElapsed.Broadcast(StageElapsedTime);
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
	OnTimeElapsed.AddUObject(this, &ThisClass::HandleTimeElapsed);

	OnAllObjectivesCleared.AddUObject(this, &ThisClass::HandleAllObjectiveCleared);
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
	if (const auto* DataSubsystem = GetCachedSubsystem<UDataSubsystem>())
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
	auto* GS = GetGameState<ARSGameState>();
	if (!GS)
	{
		return;
	}

	const auto* DataSubsystem = GetCachedSubsystem<UDataSubsystem>();
	if (!DataSubsystem)
	{
		return;
	}

	for (auto& Entry : StageObjectiveEntries)
	{
		if (const FStageObjectiveTableInfo* TableInfo = DataSubsystem->GetStageObjectiveTableInfo(Entry.TableRowName))
		{
			switch (TableInfo->ClearCondition)
			{
			case EClearCondition::KillBoss:
				// if (KillInfo->bIsBoss)
				{
					GS->SetProgress(Entry.TableRowName, KillInfo->KillCount);
				}
				break;
			case EClearCondition::KillEnemy:
				if (TableInfo->TargetTableRowName == KillInfo->MonsterRowName || TableInfo->TargetTableRowName.IsNone())
				{
					Entry.CurreentProgress = FMath::Clamp(KillInfo->KillCount, 0.0f, TableInfo->Goal);
					GS->SetProgress(Entry.TableRowName, Entry.CurreentProgress);
				}
				break;
			default:
				break;
			}
		}
	}

	CheckAllObjectiveClear();
}

void AMainGameMode::HandleTimeElapsed(float AccumulatedSec)
{
	const UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	const auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>();
	if (!DataSubsystem)
	{
		return;
	}

	for (auto& Entry : StageObjectiveEntries)
	{
		if (const FStageObjectiveTableInfo* TableInfo = DataSubsystem->GetStageObjectiveTableInfo(Entry.TableRowName))
		{
			switch (TableInfo->ClearCondition)
			{
			case EClearCondition::SurviveTime:
				if (auto* GS = GetGameState<ARSGameState>())
				{
					Entry.CurreentProgress = FMath::Clamp(AccumulatedSec, 0.0f, TableInfo->Goal);
					GS->SetProgress(Entry.TableRowName, AccumulatedSec);
				}

				CheckAllObjectiveClear();

				return;
			default:
				break;
			}
		}
	}
}

void AMainGameMode::HandleAllObjectiveCleared()
{
	//모든 좀비를 죽인다.
	StageZombieManagerComponent->AllZombieDeath();

	//모든 스폰포인트를 파괴한다.
	StageZombieManagerComponent->AllSpawnPointDestroy();

	//탈출 포탈이 없다면 생성한다.
	if (!ActivePortal)
	{
		ActivePortal = SpawnPortal(GetPortalLocation());
	}
}

void AMainGameMode::CheckAllObjectiveClear()
{
	const auto* DataSubsystem = GetCachedSubsystem<UDataSubsystem>();
	if (!DataSubsystem)
	{
		return;
	}

	for (const FStageObjectiveEntry& Objective : StageObjectiveEntries)
	{
		if (const FStageObjectiveTableInfo* Info = DataSubsystem->GetStageObjectiveTableInfo(Objective.TableRowName))
		{
			if (Objective.CurreentProgress < Info->Goal)
			{
				return;
			}
		}
	}

	OnAllObjectivesCleared.Broadcast();
}

FTransform AMainGameMode::GetPortalLocation() const
{
	if (!GameState)
	{
		return {};
	}

	for (const auto& PS : GameState->PlayerArray)
	{
		if (PS)
		{
			if (const APlayerController* PC = PS->GetPlayerController())
			{
				if (const auto* Char = PC->GetPawn<APlayerCharacterBase>())
				{
					if (IsValid(Char) && !Char->IsDead())
					{
						return Char->GetTransform();
					}
				}
			}
		}
	}

	return {};
}

ATravelPortal* AMainGameMode::SpawnPortal(const FTransform& PortalLocation) const
{
	if (!TravelPortalClass)
	{
		return nullptr;
	}

	// TSoftObjectPtr는 객체의 경로(Path)를 가리키는 포인터 역할을 하지, 메모리에 로드된 객체 자체가 아닙니다.
	// 따라서 if (!LobbyLevelAsset) 문을 작성하면 언리얼은 "이 포인터가 가리키는 UWorld 메모리 객체가 현재 로드되어 있는가?"를 검사하게 됩니다.
	if (LobbyLevelAsset.IsNull())
	{
		return nullptr;;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	if (auto* Portal = GetWorld()->SpawnActor<ATravelPortal>(TravelPortalClass, PortalLocation, Params))
	{
		FString PackagePath = LobbyLevelAsset.GetLongPackageName();
		FName LevelName = FName(*FPackageName::GetShortName(PackagePath));
		Portal->SetTarget(LevelName, PackagePath);

		return Portal;
	}

	return nullptr;
}
