// Fill out your copyright notice in the Description page of Project Settings.


#include "RSPlayerState.h"

#include "AbilitySystemComponent.h"
#include "RS/Core/LogCategory.h"
#include "RS/Core/RSUtils.h"
#include "Net/UnrealNetwork.h"
#include "RS/GameFramework/RSGameInstance.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"
#include "RS/Inventory/InventoryComponent.h"

ARSPlayerState::ARSPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);

	// APlayerState 기본값(1Hz)이 너무 낮아서, 전투 중 다른 리플리케이션(총알/HP 등)에 밀려
	// 인벤토리 아이템 서브오브젝트가 수십 초씩 지연/유실되는 문제가 있었음
	SetNetUpdateFrequency(10.0f);
	NetPriority = 3.0f;
}

void ARSPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	UE_LOG(LogTemp, Warning, TEXT("=== CopyProperties CALLED === My Texture: [%s]"), *SelectedTextureName.ToString());

	if (ARSPlayerState* NewPS = Cast<ARSPlayerState>(PlayerState))
	{
		NewPS->SelectedTextureName = SelectedTextureName;
		NewPS->SelectedSkillName = SelectedSkillName;
	}
}

void ARSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARSPlayerState, SelectedTextureName);
	DOREPLIFETIME(ARSPlayerState, SelectedSkillName);
	DOREPLIFETIME(ARSPlayerState, KillCount);
	DOREPLIFETIME(ARSPlayerState, PlayTime);
}

void ARSPlayerState::NotifyPlayerStateReady()
{
	if (bPlayerStateReady)
	{
		return;
	}

	bPlayerStateReady = true;

	OnPlayerStateReady.Broadcast();
}

UAbilitySystemComponent* ARSPlayerState::GetASC() const
{
	if (!IsValid(GetPawn()))
	{
		return nullptr;
	}

	return RSUtils::FindASC(GetPawn());
}

void ARSPlayerState::BeginPlay()
{
	Super::BeginPlay();

	InitializePlayerState();
}

void ARSPlayerState::SaveToDiskIfSinglePlayer()
{
	const auto* GI = Cast<URSGameInstance>(GetGameInstance());
	if (!GI || !GI->IsSinglePlayerMode())
	{
		return;
	}

	if (const UGameInstance* GameInst = GetGameInstance())
	{
		if (auto* StorageSubsystem = GameInst->GetSubsystem<UDataStorageSubsystem>())
		{
			StorageSubsystem->SavePlayerDataToDisk(this);
		}
	}
}

void ARSPlayerState::OnInventoryChanged()
{
	SaveToDiskIfSinglePlayer();
}

void ARSPlayerState::InitializePlayerState()
{
	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChangedDelegate.AddUObject(this, &ThisClass::OnInventoryChanged);
	}
	// 실제 PS 초기화
	NotifyPlayerStateReady();
}
