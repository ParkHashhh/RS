// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryPopupWidget.h"

#include "Components/InventoryPopupSlotObject.h"
#include "Components/StatEntryObject.h"
#include "Components/TileView.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/GameMode/GameplaySubsystem.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Player/Attribute/PlayerAttributeSet.h"

void UInventoryPopupWidget::Init(UInventoryComponent* InInventoryComponent)
{
	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Init, InInventoryComponent: %s"), *LOG_LINE_INFO,
	             *InInventoryComponent->GetName());
	InventoryComponent = InInventoryComponent;
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

	for (const FInventoryEntry& Entry : InventoryComponent->GetEntries())
	{
		auto* NewObj = NewObject<UInventoryPopupSlotObject>(this);
		NewObj->Initialize(Entry.Item);
		SlotObjects.Add(NewObj);
	}

	for (int32 i = SlotObjects.Num(); i < MaxSlotCount; i++)
	{
		auto* NewObj = NewObject<UInventoryPopupSlotObject>(this);
		NewObj->Initialize(NewObject<UItemInstance>());
		SlotObjects.Add(NewObj);
	}

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: RefreshInventory, ItemCount: %d"), *LOG_LINE_INFO, SlotObjects.Num());
	InventoryTileView->SetListItems(SlotObjects);

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

	if (InventoryComponent)
	{
		InventoryComponent->OnInventoryChangedDelegate.AddUObject(this, &ThisClass::RefreshInventory);
	}

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
	if (bGameplayReady)
	{
		if (auto* PS = GetOwningPlayerState<ARSPlayerState>())
		{
			Init(PS->GetInventoryComponent());
		}
	}
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
