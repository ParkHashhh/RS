// Fill out your copyright notice in the Description page of Project Settings.

#include "TravelPortal.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Net/UnrealNetwork.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/GameMode/RSGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"

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
	if (!GS || GS->PlayerArray.Num() == 0 || ReadyPlayers.Num() < GS->PlayerArray.Num())
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
				for (const TWeakObjectPtr<APlayerCharacterBase>& WeakPlayer : ReadyPlayers)
				{
					if (APlayerCharacterBase* Player = WeakPlayer.Get())
					{
						if (ARSPlayerState* PS = Player->GetPlayerState<ARSPlayerState>())
						{
							StorageSubsystem->MarkPendingStageResult(PS->GetUniqueId());
						}
					}
				}
			}
		}
		GI->LevelLoading(TargetLevelName, TargetLevelPackagePath, TEXT(""), false, true, true);
	}
}
