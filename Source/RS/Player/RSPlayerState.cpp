// Fill out your copyright notice in the Description page of Project Settings.


#include "RSPlayerState.h"

#include "AbilitySystemComponent.h"
#include "RS/LogCategory.h"
#include "RS/RSUtils.h"
#include "Net/UnrealNetwork.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/Inventory/InventoryComponent.h"

ARSPlayerState::ARSPlayerState()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(InventoryComponentName);
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
