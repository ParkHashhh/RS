// Fill out your copyright notice in the Description page of Project Settings.


#include "StageResultWidget.h"

#include "Components/StageResultItemSlotObject.h"
#include "Components/StageResultStatEntryObject.h"
#include "Components/TileView.h"
#include "RS/GameMode/GameplaySubsystem.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Player/ColonyPlayerController.h"

void UStageResultWidget::Init(UInventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;

	SetStatistics();
	SetNewAchievedItems();
}

void UStageResultWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* GameplaySubsystem = GetOwningLocalPlayer()->GetSubsystem<UGameplaySubsystem>();
	check(GameplaySubsystem);

	GameplaySubsystem->OnGameplayReady.AddUObject(this, &ThisClass::OnGameplayReadyChanged);
	ConfirmButton->OnClicked.AddDynamic(this, &ThisClass::OnConfirmClicked);
	OnGameplayReadyChanged(GameplaySubsystem->IsGameplayReady());
}

void UStageResultWidget::OnGameplayReadyChanged(bool bIsGameplayReady)
{
	if (bIsGameplayReady)
	{
		if (auto* PS = GetOwningPlayerState<ARSPlayerState>())
		{
			if (UInventoryComponent* InvenComp = PS->GetInventoryComponent())
			{
				Init(InvenComp);
			}
		}
	}
}

void UStageResultWidget::OnConfirmClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(true);
		PC->SetInputMode(InputMode);

		if (auto* ColonyPC = Cast<AColonyPlayerController>(PC))
		{
			ColonyPC->Server_ClearPendingStageResult();
		}
	}

	RemoveFromParent();
}

void UStageResultWidget::SetStatistics()
{
	if (!StatListView)
	{
		return;
	}

	// 현재 전체 킬, 게임 시간만 출력. 개수가 늘어나면 변경할 것
	StatObjects.Reset(2);

	StatListView->ClearListItems();

	if (const auto* PS = GetOwningPlayerState<ARSPlayerState>())
	{
		UStageResultStatEntryObject* NewObj;

		NewObj = NewObject<UStageResultStatEntryObject>(this);
		NewObj->Initialize(EStageResultStatType::Kill, PS->GetKillCount());
		StatObjects.Emplace(NewObj);

		NewObj = NewObject<UStageResultStatEntryObject>(this);
		NewObj->Initialize(EStageResultStatType::Time, PS->GetPlayTime());
		StatObjects.Emplace(NewObj);
	}

	StatListView->SetListItems(StatObjects);
}

void UStageResultWidget::SetNewAchievedItems()
{
	if (!InventoryComponent)
	{
		return;
	}

	if (!ItemSlotTileView)
	{
		return;
	}

	const TArray<FItemStorageInfo>& NewAchivedItems = InventoryComponent->GetNewAchivedItems();

	ItemSlotObjects.Reset(NewAchivedItems.Num());

	ItemSlotTileView->ClearListItems();

	for (const FItemStorageInfo& NewItem : NewAchivedItems)
	{
		if (UItemInstance* InvenItem = InventoryComponent->FindItem(NewItem.RowName))
		{
			// auto* NewObj = NewObject<UInventoryPopupSlotObject>(this);
			auto* NewObj = NewObject<UStageResultItemSlotObject>(this);
			NewObj->Initialize(NewItem);
			ItemSlotObjects.Emplace(NewObj);
		}
	}

	ItemSlotTileView->SetListItems(ItemSlotObjects);
}
