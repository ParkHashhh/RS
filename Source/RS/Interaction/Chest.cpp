// Fill out your copyright notice in the Description page of Project Settings.


#include "Chest.h"

#include "InteractableComponent.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/PlayerControllerBase.h"


AChest::AChest()
{
	PrimaryActorTick.bCanEverTick = false;

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

	// TODO: 아이템 생성
	TArray<FName> RewardItemRowNames
	{
		FName(TEXT("attack_power")), FName(TEXT("critical_chance")), FName(TEXT("hp_booster"))
	};

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

void AChest::BeginPlay()
{
	Super::BeginPlay();
}

void AChest::OnPlayerGetReward(APlayerCharacterBase* Player)
{
}
