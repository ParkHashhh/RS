// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/GameFramework/GameMode/MainGameMode.h"

#include "RS/GameFramework/GameState/RSGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "RS/Core/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Interaction/Chest.h"
#include "RS/Gameplay/Portal/PortalLocation.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Gameplay/Portal/TravelPortal.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/Zombie/Manager/ZombieManager.h"
#include "RS/Core/RSUtils.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/GameFramework/RSGameInstance.h"


AMainGameMode::AMainGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseSeamlessTravel = true;
	bStartPlayersAsSpectators = true;
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

void AMainGameMode::InitGameState()
{
	Super::InitGameState();

	StartStage();
}

void AMainGameMode::StartPlay()
{
	Super::StartPlay();
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

int32 AMainGameMode::GetTotalKillCountAll() const
{
	int32 KillCount = 0;

	for (const auto& KillInfo : KillInfos)
	{
		KillCount += KillInfo.KillCount;
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

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| OnMonsterDead CALLED, Player: %s"),
		*LOG_LINE_INFO, PlayerState ? *PlayerState->GetPlayerName() : TEXT("NULL"));
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
		Found->bIsBoss = Monster->GetIsBoss();
		Found->KillCount++;
	}
	else
	{
		Found = &KillInfos.Emplace_GetRef(FKillInfo(PlayerUID, MonsterRowName, 1));
		Found->bIsBoss = Monster->GetIsBoss();
	}

	OnKillInfoChanged.Broadcast(Found);
}

TObjectPtr<UZombieManager> AMainGameMode::GetZombieManager()
{
	return StageZombieManagerComponent;
}

int32 AMainGameMode::GetPlayerCount() const
{
	// PlayerArray.Num()은 접속만 하고 폰이 아직 없는 클라이언트도 그대로 세버려서,
	// 포탈 쪽 필요인원 카운트(TravelPortal::CheckAllPlayersReady)랑 기준이 달라 미션 목표가
	// 실제 참여 인원보다 많게 잡히는 문제가 있었다. 폰을 실제로 가진 인원만 센다.
	if (!GameState)
	{
		return 1;
	}

	int32 Count = 0;
	for (const TObjectPtr<APlayerState>& PS : GameState->PlayerArray)
	{
		if (PS && PS->GetPawn<APlayerCharacterBase>())
		{
			++Count;
		}
	}

	return FMath::Max(Count, 1);
}

float AMainGameMode::GetScaledGoal(const FStageObjectiveTableInfo* TableInfo) const
{
	return TableInfo->GetScaledGoal(GetPlayerCount());
}

void AMainGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ARSGameState* GS = GetGameState<ARSGameState>();
	if(!GS)
	{
		return;
	}

	GS->Multicast_PartyListRefresh();
}

void AMainGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ARSGameState* GS = GetGameState<ARSGameState>();
	if (!GS)
	{
		return;
	}

	GS->Multicast_PartyListRefresh();
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

		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				StorageSubsystem->RestoreDifficultyStage(GS);
			}
		}
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
	bIsCleared = false;
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
				if (KillInfo->bIsBoss)
				{
					Entry.CurreentProgress = FMath::Clamp(static_cast<float>(GetKillCountAll(KillInfo->MonsterRowName)), 0.0f, TableInfo->Goal);
					GS->SetProgress(Entry.TableRowName, Entry.CurreentProgress);

					if (Entry.CurreentProgress >= TableInfo->Goal)
					{
						if (bIsCleared)
						{
							return;
						}

						OnAllObjectivesCleared.Broadcast();
						return;
					}

				}
				break;
			case EClearCondition::KillEnemy:
				if (TableInfo->TargetTableRowName == KillInfo->MonsterRowName || TableInfo->TargetTableRowName.IsNone())
				{
					// TargetTableRowName이 None이면 "아무 몬스터나 N마리" 목표라 전체 킬 수를 합산해야 함.
					// 방금 죽은 몬스터의 RowName으로만 집계하면 다른 종류 몬스터를 죽일 때마다
					// 그 몬스터 하나만의 킬 수로 진행도가 덮어써져서 (예: 일반 좀비 10킬 -> 특수 좀비 첫 킬 시 1로 회귀) 버그가 남.
					const int32 TotalKillCount = TableInfo->TargetTableRowName.IsNone()
						? GetTotalKillCountAll()
						: GetKillCountAll(KillInfo->MonsterRowName);
					Entry.CurreentProgress = FMath::Clamp(static_cast<float>(TotalKillCount), 0.0f, GetScaledGoal(TableInfo));
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
					Entry.CurreentProgress = FMath::Clamp(AccumulatedSec, 0.0f, TableInfo->GetScaledGoal(GetPlayerCount()));
					GS->SetProgress(Entry.TableRowName, Entry.CurreentProgress);
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
	if (bIsCleared)
	{
		return;
	}
	bIsCleared = true;

	// 보스방(KillBoss 목표가 있는 스테이지)만 클리어 시 난이도가 올라간다.
	// SurviveTime/KillEnemy만 있는 파밍성 스테이지는 클리어해도 난이도를 유지한다.
	bool bHasBossObjective = false;
	if (const auto* DataSubsystem = GetCachedSubsystem<UDataSubsystem>())
	{
		for (const FStageObjectiveEntry& Objective : StageObjectiveEntries)
		{
			if (const FStageObjectiveTableInfo* Info = DataSubsystem->GetStageObjectiveTableInfo(Objective.TableRowName))
			{
				if (Info->ClearCondition == EClearCondition::KillBoss)
				{
					bHasBossObjective = true;
					break;
				}
			}
		}
	}

	if (bHasBossObjective)
	{
	if (auto* GS = GetGameState<ARSGameState>())
	{
		GS->SetDifficultyStage(GS->GetDifficultyStage() + 1);

		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				// CurrentDifficultyStage는 스테이지->콜로니->스테이지로 오갈 때 GameState가 매번
				// 새로 생성되는 걸 버텨내려고 GameInstance에 들고 있는 값이라, 싱글/멀티 여부와 무관하게
				// 항상 갱신해야 함. 예전엔 싱글플레이 세이브(SavePlayerDataToDisk)랑 묶여있어서 멀티플레이에서는
				// 이 값이 영원히 1에 머물렀고, 그래서 보스를 잡아도 맵선택 화면의 "Level"이 안 올라갔음.
				// 디스크 저장은 싱글플레이에서만 의미 있으니 그것만 계속 게이팅한다.
				StorageSubsystem->SetCurrentDifficultyStage(GS->GetDifficultyStage());

				if (const auto* RSGI = Cast<URSGameInstance>(GI))
				{
					if (RSGI->IsSinglePlayerMode())
					{
						if (GameState->PlayerArray.IsValidIndex(0))
						{
							if (auto* PS = Cast<ARSPlayerState>(GameState->PlayerArray[0]))
							{
								StorageSubsystem->SavePlayerDataToDisk(PS);
							}
						}
					}
				}
			}
		}
	}
	}

	//모든 좀비를 죽인다.
	StageZombieManagerComponent->AllZombieDeath();

	//모든 스폰포인트를 파괴한다.
	StageZombieManagerComponent->AllSpawnPointDestroy();

	ActiveChest();

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
			if (Info->ClearCondition == EClearCondition::KillBoss)
			{
				return;
			}
		}
	}

	for (const FStageObjectiveEntry& Objective : StageObjectiveEntries)
	{
		if (const FStageObjectiveTableInfo* Info = DataSubsystem->GetStageObjectiveTableInfo(Objective.TableRowName))
		{
			if (Objective.CurreentProgress < GetScaledGoal(Info))
			{
				return;
			}
		}
	}

	OnAllObjectivesCleared.Broadcast();
}

FTransform AMainGameMode::GetPortalLocation() const
{
	if (const AActor* PortalLocActor = UGameplayStatics::GetActorOfClass(this, APortalLocation::StaticClass()))
	{
		return PortalLocActor->GetTransform();
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

void AMainGameMode::ActiveChest()
{
	if (!bScaleChestsByPlayerCount)
	{
		return;
	}

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChest::StaticClass(), Actors);

	const int32 NumToActivate = FMath::Min(GetPlayerCount() * 2, Actors.Num());

	RS_LOG_DEBUG(LogRSSession, Warning,
		TEXT("%s: ActiveChest, PlayerCount: %d, TotalChestActors: %d, NumToActivate: %d"),
		*LOG_LINE_INFO, GetPlayerCount(), Actors.Num(), NumToActivate);

	for (int32 Index = 0; Index < Actors.Num(); ++Index)
	{
		if (auto* Chest = Cast<AChest>(Actors[Index]))
		{
			Chest->SetActive(Index < NumToActivate);
		}
	}
}

void AMainGameMode::CheckAllPlayersDead()
{
	if (!GameState || GameState->PlayerArray.Num() == 0)
	{
		return;
	}

	for (const APlayerState* PS : GameState->PlayerArray)
	{
		if (const APlayerController* PC = PS->GetPlayerController())
		{
			if (const auto* Char = PC->GetPawn<APlayerCharacterBase>())
			{
				if (!Char->IsDead())
				{
					return;   // 살아있는 사람 있으면 아직 전멸 아님
				}
			}
		}
	}

	HandleAllPlayersDead();
}

void AMainGameMode::HandleAllPlayersDead()
{
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				StorageSubsystem->ClearPlayerInventory(PS->GetUniqueId());
				StorageSubsystem->DeletePlayerSaveFile(PS->GetUniqueId());

				if (auto* RSPS = Cast<ARSPlayerState>(PS))
				{
					RSPS->LoadSavedNames(NAME_None, NAME_None);
				}
			}
		}
	}

	RSUtils::ServerTravelWithDataStorage(GetWorld(), TEXT("Start"), false);
}
