// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryPopupWidget.h"

#include "GameplayTagContainer.h"
#include "Components/InventoryPopupSlotObject.h"
#include "Components/StatEntryObject.h"
#include "Components/TileView.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Inventory/InventoryComponent.h"
#include "RS/Player/PlayerCharacterBase.h"
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

	SlotObjects.Reset();

	InventoryTileView->ClearListItems();

	for (auto& Entry : InventoryComponent->GetEntries())
	{
		auto* NewObj = NewObject<UInventoryPopupSlotObject>(this);
		NewObj->Initialize(Entry.Item);
		SlotObjects.Add(NewObj);
	}

	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: RefreshInventory, ItemCount: %d"), *LOG_LINE_INFO, SlotObjects.Num());
	InventoryTileView->SetListItems(SlotObjects);

	// 스탯 및 특수 효과 추가
	RefreshStatList();
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

	auto* PlayerCharacter = GetOwningPlayerPawn<APlayerCharacterBase>();
	if (!PlayerCharacter)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get PlayerCharacter"), *LOG_LINE_INFO);
		return;
	}

	UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
	if (!ASC)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get ASC"), *LOG_LINE_INFO);
		return;
	}

	auto* Attr = Cast<UPlayerAttributeSet>(ASC->GetAttributeSet(UPlayerAttributeSet::StaticClass()));
	if (!Attr)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Failed to get Attr"), *LOG_LINE_INFO);
		return;
	}

	UStatEntryObject* StatEntryObject;
	const auto* ContextString = TEXT("UInventoryPopupWidget::AddBasicStats");

	if (auto Info = StatDisplayInfoTable->FindRow<FStatDisplayTableInfo>(FName(TEXT("max_hp")), ContextString))
	{
		StatEntryObject = NewObject<UStatEntryObject>(this);
		StatEntryObject->Initialize(Info->DisplayName, RSUtils::GetStatDescription(*Info, {Attr->GetMaxHP()}),
		                            Info->Icon);
		StatObjects.Add(StatEntryObject);
	}

	if (auto Info = StatDisplayInfoTable->FindRow<FStatDisplayTableInfo>(FName(TEXT("attack_power")), ContextString))
	{
		StatEntryObject = NewObject<UStatEntryObject>(this);
		StatEntryObject->Initialize(Info->DisplayName, RSUtils::GetStatDescription(*Info, {Attr->GetAttackPower()}),
		                            Info->Icon);
		StatObjects.Add(StatEntryObject);
	}

	if (auto Info = StatDisplayInfoTable->FindRow<FStatDisplayTableInfo>(FName(TEXT("attack_speed")), ContextString))
	{
		StatEntryObject = NewObject<UStatEntryObject>(this);
		StatEntryObject->Initialize(Info->DisplayName, RSUtils::GetStatDescription(*Info, {Attr->GetAttackSpeed() * 100.0f}),
									Info->Icon);
		StatObjects.Add(StatEntryObject);
	}

	if (auto Info = StatDisplayInfoTable->FindRow<FStatDisplayTableInfo>(FName(TEXT("critical_chance")), ContextString))
	{
		StatEntryObject = NewObject<UStatEntryObject>(this);
		StatEntryObject->Initialize(Info->DisplayName, RSUtils::GetStatDescription(*Info, {Attr->GetCriticalChance() * 100.0f}),
									Info->Icon);
		StatObjects.Add(StatEntryObject);
	}

	if (auto Info = StatDisplayInfoTable->FindRow<FStatDisplayTableInfo>(FName(TEXT("critical_damage")), ContextString))
	{
		StatEntryObject = NewObject<UStatEntryObject>(this);
		StatEntryObject->Initialize(Info->DisplayName, RSUtils::GetStatDescription(*Info, {Attr->GetCriticalDamage() * 100.0f}),
									Info->Icon);
		StatObjects.Add(StatEntryObject);
	}

	// TODO: 이후로 추가
}

void UInventoryPopupWidget::AddItemEffects()
{
}

void UInventoryPopupWidget::AddStat(const FGameplayTag& StatTag, const FText& StatName, const FText& StatDescription)
{
}
