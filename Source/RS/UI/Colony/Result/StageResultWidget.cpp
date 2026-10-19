// Fill out your copyright notice in the Description page of Project Settings.


#include "StageResultWidget.h"

#include "Components/StageResultItemSlotObject.h"
#include "Components/StageResultStatEntryObject.h"
#include "Components/TileView.h"
#include "RS/Player/Subsystem/GameplaySubsystem.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Player/Controller/ColonyPlayerController.h"
#include "RS/Core/LogCategory.h"

void UStageResultWidget::Init(UInventoryComponent* InInventoryComponent)
{
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
	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: OnGameplayReadyChanged, bIsGameplayReady: %d"),
		*LOG_LINE_INFO, bIsGameplayReady);

	if (bIsGameplayReady)
	{
		if (bAlreadyInitialized)
		{
			RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: SKIPPED, already initialized"), *LOG_LINE_INFO);
			return;
		}

		if (auto* PS = GetOwningPlayerState<ARSPlayerState>())
		{
			if (UInventoryComponent* InvenComp = PS->GetInventoryComponent())
			{
				RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: Calling Init, InvenComp: %s, EntryCount: %d"),
					*LOG_LINE_INFO, *InvenComp->GetName(), InvenComp->GetEntryCount());
				bAlreadyInitialized = true;
				Init(InvenComp);
			}
			else
			{
				RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: InventoryComponent is nullptr"), *LOG_LINE_INFO);
			}
		}
		else
		{
			RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: OwningPlayerState is nullptr"), *LOG_LINE_INFO);
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

	// ARSPlayerState::KillCount/PlayTime은 OnRep 없는 리플리케이트 프로퍼티라 Client_ShowStageResultWidget
	// RPC보다 늦게 도착할 수 있음 - RPC가 실어온 값(ColonyPlayerController에 보관)을 대신 사용한다.
	if (const auto* ColonyPC = Cast<AColonyPlayerController>(GetOwningPlayer()))
	{
		UStageResultStatEntryObject* NewObj;

		NewObj = NewObject<UStageResultStatEntryObject>(this);
		NewObj->Initialize(EStageResultStatType::Kill, ColonyPC->GetPendingStageResultKillCount());
		StatObjects.Emplace(NewObj);

		NewObj = NewObject<UStageResultStatEntryObject>(this);
		NewObj->Initialize(EStageResultStatType::Time, ColonyPC->GetPendingStageResultPlayTime());
		StatObjects.Emplace(NewObj);
	}

	StatListView->SetListItems(StatObjects);
}

void UStageResultWidget::SetNewAchievedItems()
{
	if (!ItemSlotTileView)
	{
		return;
	}

	// UInventoryComponent::GetNewAchivedItems()는 로컬(호출한 머신의) UDataStorageSubsystem에 저장된
	// Previous/PrePreviousInventoryStorage를 읽는데, 이 스냅샷은 서버(호스트)에서만 채워짐 - 원격 클라
	// 자신의 로컬 서브시스템엔 그 데이터가 없어서 직접 계산하면 diff 기준이 항상 비어있어 이전에 먹은
	// 아이템까지 전부 "새 아이템"으로 잡힘. 서버가 계산해서 Client_ShowStageResultWidget RPC로 실어보낸 값을 쓴다.
	const auto* ColonyPC = Cast<AColonyPlayerController>(GetOwningPlayer());
	const TArray<FItemStorageInfo> NewAchivedItems = ColonyPC ? ColonyPC->GetPendingStageResultNewItems() : TArray<FItemStorageInfo>();

	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: NewAchivedItems.Num(): %d"), *LOG_LINE_INFO, NewAchivedItems.Num());

	ItemSlotObjects.Reset(NewAchivedItems.Num());

	ItemSlotTileView->ClearListItems();

	for (const FItemStorageInfo& NewItem : NewAchivedItems)
	{
		// NewItem(RowName/Count)만으로 슬롯을 채울 수 있음 - 원격 클라에서 UItemInstance 서브오브젝트가
		// 아직 리플리케이션 안 됐거나 영영 안 오더라도 결과창에는 항상 표시되도록 FindItem 존재 확인을 하지 않는다.
		RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: Added ItemSlot, Row: %s, Count: %d"),
			*LOG_LINE_INFO, *NewItem.RowName.ToString(), NewItem.Count);
		auto* NewObj = NewObject<UStageResultItemSlotObject>(this);
		NewObj->Initialize(NewItem);
		ItemSlotObjects.Emplace(NewObj);
	}

	ItemSlotTileView->SetListItems(ItemSlotObjects);
}
