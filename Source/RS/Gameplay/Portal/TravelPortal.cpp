// Fill out your copyright notice in the Description page of Project Settings.

#include "RS/Gameplay/Portal/TravelPortal.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/GameFramework/RSGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Core/LogCategory.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"

ATravelPortal::ATravelPortal()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	SetRootComponent(CollisionComp);
	CollisionComp->SetCollisionProfileName(TEXT("Portal"));
	CollisionComp->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalVFX"));
	NiagaraComp->SetupAttachment(CollisionComp);
	NiagaraComp->SetAutoActivate(true);

	RangeNiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RangeIndicatorComp"));
	RangeNiagaraComp->SetupAttachment(CollisionComp);
	RangeNiagaraComp->SetAutoActivate(true);
}

void ATravelPortal::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ATravelPortal::OnOverlapBegin);
		CollisionComp->OnComponentEndOverlap.AddDynamic(this, &ATravelPortal::OnOverlapEnd);
	}
}

void ATravelPortal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATravelPortal, TargetLevelName);
	DOREPLIFETIME(ATravelPortal, TargetLevelPackagePath);
}

void ATravelPortal::SetTarget(FName InLevelName, const FString& InPackagePath)
{
	TargetLevelName = InLevelName;
	TargetLevelPackagePath = InPackagePath;
}

void ATravelPortal::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(OtherActor))
	{
		ReadyPlayers.Add(Player);
		CheckAllPlayersReady();
	}
}

void ATravelPortal::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(OtherActor))
	{
		ReadyPlayers.Remove(Player);
		if (GetWorldTimerManager().IsTimerActive(DepartureTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(DepartureTimerHandle);
		}
	}
}

void ATravelPortal::CheckAllPlayersReady()
{
	AGameStateBase* GS = GetWorld()->GetGameState();
	if (!GS || GS->PlayerArray.Num() == 0)
	{
		return;
	}

	// 관전자(죽은 플레이어)는 포탈에 물리적으로 들어올 수 없으니 필요 인원에서 제외
	int32 RequiredPlayerCount = 0;
	for (const TObjectPtr<APlayerState>& PS : GS->PlayerArray)
	{
		const APlayerCharacterBase* Player = PS ? PS->GetPawn<APlayerCharacterBase>() : nullptr;
		if (Player && !Player->IsDead())
		{
			++RequiredPlayerCount;
		}
	}

	if (RequiredPlayerCount == 0 || ReadyPlayers.Num() < RequiredPlayerCount)
	{
		return;
	}
	if (GetWorldTimerManager().IsTimerActive(DepartureTimerHandle))
	{
		return;
	}

	GetWorldTimerManager().SetTimer(DepartureTimerHandle, this, &ATravelPortal::ExecuteTravel, DepartureDelay, false);
}
void ATravelPortal::ExecuteTravel()
{
	if (URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance()))
	{
		if (TargetLevelName == FName(TEXT("Colony")))
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				// ReadyPlayers는 포탈에 물리적으로 올라온(=살아있는) 플레이어만 담고 있어서,
				// 죽어서 관전 중인 플레이어는 여기서 걸러지면 결과창 자체가 안 뜬다.
				// 콜로니로 돌아갈 땐 생사 무관하게 접속 중인 전원한테 결과창을 띄워야 하니
				// ReadyPlayers 대신 GameState의 전체 PlayerArray를 순회한다.
				if (const AGameStateBase* GS = GetWorld()->GetGameState())
				{
					for (const TObjectPtr<APlayerState>& PS : GS->PlayerArray)
					{
						if (const ARSPlayerState* RSPS = Cast<ARSPlayerState>(PS))
						{
							StorageSubsystem->MarkPendingStageResult(RSPS->GetUniqueId());
						}
					}
				}
			}
		}
		GI->LevelLoading(TargetLevelName, TargetLevelPackagePath, TEXT(""), false, true, true);
	}
}
