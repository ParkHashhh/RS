// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/GameFramework/GameMode/RSInGameMode.h"
#include "RS/Core/LogCategory.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/Controller/PlayerControllerBase.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Gameplay/Portal/TravelPortal.h"
#include "RS/Player/Controller/ColonyPlayerController.h"
#include "RS/Inventory/InventoryComponent.h"


ARSInGameMode::ARSInGameMode()
{
	bUseSeamlessTravel = true;
}

UClass* ARSInGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	UE_LOG(LogTemp, Warning, TEXT("=== GetDefaultPawnClassForController CALLED ==="));
	if (ARSPlayerState* PS = InController->GetPlayerState<ARSPlayerState>())
	{
		FString TextureNameStr = PS->GetTextureName().ToString();
		RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| TextureNameStr: %s"), *LOG_LINE_INFO, *TextureNameStr);

		if (TextureNameStr.Contains(TEXT("SurvivalMan")))
		{
			RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| ClassName: %s"), *LOG_LINE_INFO, *MaleCharacterClass->GetName());
			return MaleCharacterClass;
		}
		else if (TextureNameStr.Contains(TEXT("SurvivalGirl")))
		{
			RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| ClassName: %s"), *LOG_LINE_INFO, *MaleCharacterClass->GetName());
			return FeMaleCharacterClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


void ARSInGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Handling starting new player"), *LOG_LINE_INFO);

	if (auto* PC = Cast<ARSPlayerController>(NewPlayer))
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				// (되돌림) 여기서 지연시키면 바로 아래 ShowStageResultWidget이 복원 전 빈 인벤토리를 읽어버려서
				// ResultWidget의 "새로 얻은 아이템" 표시가 호스트 포함 항상 비어버렸음.
				// 서브오브젝트 리플리케이션 레이스는 HandleEntryAdded 쪽 재시도로 처리하는 게 맞음.
				StorageSubsystem->RestorePlayerInventory(PC);
				StorageSubsystem->RestorePlayerStageResultStat(PC);
				if (const auto* PS = PC->GetPlayerState<ARSPlayerState>())
				{
					if (StorageSubsystem->HasPendingStageResult(PS->GetUniqueId()))
					{
						if (auto* ColonyPC = Cast<AColonyPlayerController>(NewPlayer))
						{
							// KillCount/PlayTime/새로 얻은 아이템 목록을 RPC 파라미터로 직접 실어 보낸다.
							// GetNewAchivedItems()는 UDataStorageSubsystem의 Previous/PrePreviousInventoryStorage를
							// 읽는데, 이 맵은 서버(호스트)에서만 채워짐 - 원격 클라 본인의 로컬 서브시스템엔 그 데이터가
							// 아예 없어서(replicate 안 되는 GameInstanceSubsystem) 클라가 직접 계산하면 항상 diff 기준이
							// 비어있어 이전에 먹은 아이템까지 전부 "새 아이템"으로 나옴. 서버에서 계산해서 값만 넘긴다.
							TArray<FItemStorageInfo> NewItems;
							if (UInventoryComponent* InvenComp = PS->GetInventoryComponent())
							{
								NewItems = InvenComp->GetNewAchivedItems();
							}
							ColonyPC->Client_ShowStageResultWidget(PS->GetKillCount(), PS->GetPlayTime(), NewItems);
						}
					}
				}
			}
		}
	}
}

APawn* ARSInGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| NewPlayer: %s, SpawnLoc: %s"),
		*LOG_LINE_INFO, *GetNameSafe(NewPlayer), *SpawnTransform.GetLocation().ToString());

	APawn* SpawnedPawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| SpawnResult: %s"),
		*LOG_LINE_INFO, SpawnedPawn ? *SpawnedPawn->GetName() : TEXT("NULL"));

	return SpawnedPawn;
}

void ARSInGameMode::RestartPlayer(AController* NewPlayer)
{
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| RestartPlayer CALLED for: %s"),
		*LOG_LINE_INFO, *GetNameSafe(NewPlayer));

	Super::RestartPlayer(NewPlayer);

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| RestartPlayer FINISHED for: %s, Pawn: %s"),
		*LOG_LINE_INFO, *GetNameSafe(NewPlayer),
		NewPlayer ? *GetNameSafe(NewPlayer->GetPawn()) : TEXT("N/A"));
}

bool ARSInGameMode::MustSpectate_Implementation(APlayerController* NewPlayerController) const
{
	bool bResult = Super::MustSpectate_Implementation(NewPlayerController);
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| MustSpectate: %s -> %s"),
		*LOG_LINE_INFO, *GetNameSafe(NewPlayerController), bResult ? TEXT("true") : TEXT("false"));
	return bResult;
}

bool ARSInGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	bool bHasLoaded = Player && Player->HasClientLoadedCurrentWorld();
	bool bResult = Super::PlayerCanRestart_Implementation(Player);

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| PlayerCanRestart: %s -> HasClientLoadedCurrentWorld=%s, Result=%s"),
		*LOG_LINE_INFO, *GetNameSafe(Player),
		bHasLoaded ? TEXT("true") : TEXT("false"),
		bResult ? TEXT("true") : TEXT("false"));
		
	return bResult;
}

void ARSInGameMode::CreateOrUpdatePortal(FName LevelName, const FString& PackagePath)
{
	if (IsValid(ActivePortal))
	{
		ActivePortal->Destroy();
		ActivePortal = nullptr;
	}

	if (IsValid(TravelPortalClass))
	{
		ActivePortal = GetWorld()->SpawnActor<ATravelPortal>(TravelPortalClass, PortalSpawnTransform);
		if (IsValid(ActivePortal))
		{
			ActivePortal->SetTarget(LevelName, PackagePath);
		}
	}
}
