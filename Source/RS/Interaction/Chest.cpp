// Fill out your copyright notice in the Description page of Project Settings.


#include "Chest.h"

#include "InteractableComponent.h"
#include "RS/Core/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/GameFramework/GameState/RSGameState.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/Controller/PlayerControllerBase.h"
#include "Net/UnrealNetwork.h"

AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	Interactable = CreateDefaultSubobject<UInteractableComponent>(TEXT("Interactable"));
	SetRootComponent(Interactable);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void AChest::OnInteractionCompleted(AActor* Interactor)
{
	RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: 상자 열림"), *LOG_LINE_INFO);

	if (!HasAuthority())
	{
		return;
	}

	// TODO: 상자 열림 연출

	const UGameInstance* GI = GetGameInstance();
	const auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>();
	if (!DataSubsystem)
	{
		return;
	}

	int32 CurrentStage = 1;
	if (const auto* GS = GetWorld()->GetGameState<ARSGameState>())
	{
		CurrentStage = GS->GetDifficultyStage();
	}

	const TArray<FName> RewardItemRowNames = DataSubsystem->GetThreeRandomItmes(CurrentStage);

	if (const auto* InteractPlayer = Cast<APlayerCharacterBase>(Interactor))
	{
		if (auto* PC = InteractPlayer->GetController<APlayerControllerBase>())
		{
			PC->Client_ShowRewardUI(RewardItemRowNames);
		}

		InteractPlayer->OnHeldInteractableChanged.Broadcast(true);
	}
	// TODO: 플레이어-상자상태(열기전/열었음/획득완료)-확정된 아이템-을 배열로 저장. 모든 플레이어가 획득 완료하면 상자 파괴

}

float AChest::GetInteractionDuration() const
{
	return OpenTime;
}

void AChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChest, bChestActiveState);
}

void AChest::OnRep_ChestActiveState()
{
	RS_LOG_DEBUG(LogRSInteraction, Warning, TEXT("%s: OnRep_ChestActiveState, Chest: %s, bChestActiveState: %d"),
		*LOG_LINE_INFO, *GetName(), bChestActiveState);

	SetActive(bChestActiveState);
}

void AChest::SetActive(bool bActive)
{
	bChestActiveState = bActive;

	RS_LOG_DEBUG(LogRSInteraction, Warning, TEXT("%s: SetActive, Chest: %s, HasAuthority: %d, bActive: %d"),
		*LOG_LINE_INFO, *GetName(), HasAuthority(), bActive);

	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);

	if (bActive)
	{
		Mesh->Activate();
		Interactable->Activate();
		Interactable->SetCollisionProfileName(FName("Interaction"));
	}
	else
	{
		Mesh->Deactivate();
		Interactable->SetCollisionProfileName(FName("NoCollision"));
		Interactable->Deactivate();
	}
}

void AChest::BeginPlay()
{
	Super::BeginPlay();

	// OnRep_ChestActiveState는 리플리케이트된 값이 CDO 기본값(true)과 같으면 안 불림 -
	// 그러면 클라는 SetActive를 한 번도 못 불러서 에디터에 배치된 raw 비주얼 상태로 남음. 스폰 시 강제로 한 번 적용.
	SetActive(bChestActiveState);
}

void AChest::OnPlayerGetReward(APlayerCharacterBase* Player)
{
}
