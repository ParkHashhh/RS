// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryPopupWidget.h"

#include "Components/InventoryPopupSlotObject.h"
#include "Components/StatEntryObject.h"
#include "Components/TileView.h"
#include "RS/Core/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/Subsystem/GameplaySubsystem.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Player/GAS/Attribute/PlayerAttributeSet.h"

void UInventoryPopupWidget::Init(UInventoryComponent* InInventoryComponent)
{
	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Init, InInventoryComponent: %s"), *LOG_LINE_INFO,
		*InInventoryComponent->GetName());

	if (InventoryComponent != InInventoryComponent)
	{
		InventoryComponent = InInventoryComponent;
		InventoryComponent->OnInventoryChangedDelegate.AddUObject(this, &ThisClass::RefreshInventory);
	}

	RefreshInventory();
}

void UInventoryPopupWidget::RefreshInventory()
{
	if (!InventoryComponent)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: InventoryComponent is nullptr"), *LOG_LINE_INFO)
		return;
	}
	if (!InventoryTileView)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: InventoryTileView is nullptr"), *LOG_LINE_INFO)
		return;
	}

	SlotObjects.Reset(MaxSlotCount);

	InventoryTileView->ClearListItems();

	bool bAnyEntryItemMissing = false;
	int32 MissingIndex = -1;
	int32 EntryIdx = 0;
	for (const FInventoryEntry& Entry : InventoryComponent->GetEntries())
	{
		if (!Entry.Item)
		{
			// Entry.Item 서브오브젝트 참조가 영구 유실된 경우, RPC로 받아둔 RowName/Count 백업 데이터로 대체 표시
			FItemStorageInfo FallbackInfo;
			if (InventoryComponent->GetFallbackItemData(EntryIdx, FallbackInfo))
			{
				const UGameInstance* GI = GetGameInstance();
				if (const auto* DataSubsystem = GI ? GI->GetSubsystem<UDataSubsystem>() : nullptr)
				{
					if (const UDataTable* ItemTable = DataSubsystem->GetItemInfoTable())
					{
						FDataTableRowHandle RowHandle;
						RowHandle.DataTable = ItemTable;
						RowHandle.RowName = FallbackInfo.RowName;

						auto* FallbackItem = NewObject<UItemInstance>(this);
						FallbackItem->Initialize(nullptr, FGuid(), RowHandle, FallbackInfo.Count);

						auto* NewObj = NewObject<UInventoryPopupSlotObject>(this);
						NewObj->Initialize(FallbackItem);
						SlotObjects.Add(NewObj);

						RS_LOG_DEBUG(LogRSInventory, Warning,
							TEXT("%s: Entry.Item still null, rendered from RPC fallback - EntryIdx: %d, Row: %s, Count: %d"),
							*LOG_LINE_INFO, EntryIdx, *FallbackInfo.RowName.ToString(), FallbackInfo.Count);

						++EntryIdx;
						continue;
					}

					RS_LOG_DEBUG(LogRSInventory, Error,
						TEXT("%s: Have fallback data but ItemInfoTable is nullptr, EntryIdx: %d"),
						*LOG_LINE_INFO, EntryIdx);
				}
			}

			// 배열 엔트리는 리플리케이트됐는데 그 안의 서브오브젝트 포인터가 아직 안 풀린 경우 - 다음 틱에 재시도
			bAnyEntryItemMissing = true;
			MissingIndex = EntryIdx;
			++EntryIdx;
			continue;
		}

		auto* NewObj = NewObject<UInventoryPopupSlotObject>(this);
		NewObj->Initialize(Entry.Item);
		SlotObjects.Add(NewObj);
		++EntryIdx;
	}

	if (bAnyEntryItemMissing)
	{
		RS_LOG_DEBUG(LogRSInventory, Warning,
			TEXT("%s: Entry.Item missing detail - TotalEntries: %d, ResolvedCount: %d, LastMissingIndex: %d"),
			*LOG_LINE_INFO, InventoryComponent->GetEntries().Num(), SlotObjects.Num(), MissingIndex);
	}

	for (int32 i = SlotObjects.Num(); i < MaxSlotCount; i++)
	{
		auto* NewObj = NewObject<UInventoryPopupSlotObject>(this);
		NewObj->Initialize(NewObject<UItemInstance>());
		SlotObjects.Add(NewObj);
	}

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: RefreshInventory, ItemCount: %d"), *LOG_LINE_INFO, SlotObjects.Num());
	InventoryTileView->SetListItems(SlotObjects);

	// 프레임당 재시도는 고프레임에서 200ms도 안 돼 네트워크 왕복을 못 버팀 - 0.1초 간격으로 최대 5초까지 재시도
	constexpr int32 MaxRefreshRetryCount = 50;
	constexpr float RefreshRetryInterval = 0.1f;
	if (bAnyEntryItemMissing && RefreshRetryCount < MaxRefreshRetryCount)
	{
		++RefreshRetryCount;
		RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: Entry.Item not resolved yet, retry %d"),
			*LOG_LINE_INFO, RefreshRetryCount);
		if (GetWorld())
		{
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, this, &ThisClass::RefreshInventory,
				RefreshRetryInterval, false);
		}
	}
	else
	{
		RefreshRetryCount = 0;
	}

	// 스탯 및 특수 효과 추가
	RefreshStatList();
}

void UInventoryPopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	auto* GameplaySubsystem = GetOwningLocalPlayer()->GetSubsystem<UGameplaySubsystem>();
	check(GameplaySubsystem);

	GameplaySubsystem->OnGameplayReady.AddUObject(this, &ThisClass::OnGameplayReadyChanged);
	OnGameplayReadyChanged(GameplaySubsystem->IsGameplayReady());
}

void UInventoryPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshInventory();
}

void UInventoryPopupWidget::RefreshStatList()
{
	StatObjects.Reset();
	StatListView->ClearListItems();

	AddBasicStats();
	AddItemEffects();

	StatListView->SetListItems(StatObjects);
}

void UInventoryPopupWidget::AddBasicStats()
{
	if (!InventoryComponent)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	check(GI);

	auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	const UDataTable* StatDisplayInfoTable = DataSubsystem->GetStatDisplayInfoTable();
	if (!StatDisplayInfoTable)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get StatDisplayInfoTable"), *LOG_LINE_INFO);
		return;
	}

	for (const auto& RowName : StatDisplayInfoTable->GetRowNames())
	{
		FGameplayTag Tag = FGameplayTag::RequestGameplayTag(RowName);

		if (const auto* Info = StatDisplayInfoTable->FindRow<FStatDisplayTableInfo>(
			RowName, TEXT("UInventoryPopupWidget::AddBasicStats")))
		{
			UStatEntryObject* StatEntryObject = NewObject<UStatEntryObject>(this);

			float Stat = GetAttributeValueWithTag(Tag);

			StatEntryObject->Initialize(Info->DisplayName,
			                            RSUtils::GetStatDescription(*Info, {Stat}), Info->Icon);
			StatObjects.Add(StatEntryObject);
		}
	}
}

void UInventoryPopupWidget::AddItemEffects()
{
}

void UInventoryPopupWidget::AddStat(const FGameplayTag& StatTag, const FText& StatName, const FText& StatDescription)
{
}

void UInventoryPopupWidget::OnGameplayReadyChanged(bool bGameplayReady)
{
	if (!bGameplayReady)
	{
		return;
	}

	auto* PS = GetOwningPlayerState<ARSPlayerState>();
	if (!PS)
	{
		// GameplayReady는 켜졌는데 PlayerState가 아직 이 위젯에 안 붙었을 수 있음(레벨 전환 직후 등) - 다음 틱에 재시도
		constexpr int32 MaxInitRetryCount = 30;
		if (InitRetryCount < MaxInitRetryCount && GetWorld())
		{
			++InitRetryCount;
			RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: OwningPlayerState not ready yet, retry %d"),
				*LOG_LINE_INFO, InitRetryCount);
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::RetryInit);
		}
		return;
	}

	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: OwningPlayerState resolved after %d retries, PS: %s"),
		*LOG_LINE_INFO, InitRetryCount, *GetNameSafe(PS));

	InitRetryCount = 0;
	Init(PS->GetInventoryComponent());
}

void UInventoryPopupWidget::RetryInit()
{
	auto* GameplaySubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UGameplaySubsystem>() : nullptr;
	OnGameplayReadyChanged(GameplaySubsystem && GameplaySubsystem->IsGameplayReady());
}

float UInventoryPopupWidget::GetAttributeValueWithTag(const FGameplayTag& TargetTag) const
{
	// 전체 attribute. 일단 EquipItemEffectClass을 사용
	const UGameplayEffect* GE = EquipItemEffectClass.GetDefaultObject();
	if (!GE)
	{
		return 0.0f;
	}

	for (const FGameplayModifierInfo& Modifier : GE->Modifiers)
	{
		if (Modifier.ModifierMagnitude.GetMagnitudeCalculationType()
			!= EGameplayEffectMagnitudeCalculation::SetByCaller)
		{
			continue;
		}

		const FSetByCallerFloat& SetByCaller = Modifier.ModifierMagnitude.GetSetByCallerFloat();

		if (SetByCaller.DataTag == TargetTag)
		{
			FGameplayAttribute TargetAttribute = Modifier.Attribute;

			if (auto* PlayerCharacter = GetOwningPlayerPawn<APlayerCharacterBase>())
			{
				if (UPlayerAttributeSet* AttributeSet = PlayerCharacter->GetAttributeSet())
				{
					return TargetAttribute.GetNumericValue(AttributeSet);
				}
			}

			return 0.0f;
		}
	}

	return 0.0f;
}
