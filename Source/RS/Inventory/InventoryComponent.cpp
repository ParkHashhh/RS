// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "RS/LogCategory.h"
#include "RS/RSTypes.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/RSPlayerState.h"

void FInventoryEntry::PreReplicatedRemove(const FInventoryList& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->HandleEntryRemoved(*this);
	}
}

void FInventoryEntry::PostReplicatedAdd(const FInventoryList& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->HandleEntryAdded(*this);
	}
}

void FInventoryEntry::PostReplicatedChange(const FInventoryList& InArraySerializer)
{
	if (InArraySerializer.Owner)
	{
		InArraySerializer.Owner->HandleEntryChanged(*this);
	}
}

void FInventoryList::RebuildIndexMap()
{
	GuidIndexMap.Empty(Entries.Num());

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		if (!Entries[Index].Item)
		{
			continue;
		}

		GuidIndexMap.Add(Entries[Index].Item->GetItemGUID(), Index);
	}
}

void FInventoryList::MarkDirty(const UItemInstance* Item)
{
	if (!Item)
	{
		return;
	}

	if (FInventoryEntry* Entry = FindEntry(Item->GetItemGUID()))
	{
		MarkItemDirty(*Entry);
	}
}

FInventoryEntry* FInventoryList::FindEntry(const FGuid& Guid)
{
	const int32* Index = GuidIndexMap.Find(Guid);
	if (!Index)
	{
		return nullptr;
	}

	return &Entries[*Index];
}

const FInventoryEntry* FInventoryList::FindEntry(const FGuid& Guid) const
{
	const int32* Index = GuidIndexMap.Find(Guid);
	if (!Index)
	{
		return nullptr;
	}

	return &Entries[*Index];
}

FInventoryEntry* FInventoryList::FindEntry(const FDataTableRowHandle& RowHandle)
{
	return Entries.FindByPredicate(
		[&](const FInventoryEntry& Entry)
		{
			return Entry.Item && Entry.Item->GetItemRowHandle() == RowHandle;
		});
}

const FInventoryEntry* FInventoryList::FindEntry(const FDataTableRowHandle& RowHandle) const
{
	return Entries.FindByPredicate(
		[&](const FInventoryEntry& Entry)
		{
			return Entry.Item && Entry.Item->GetItemRowHandle() == RowHandle;
		});
}

FInventoryEntry* FInventoryList::FindEntry(FName ItemRowName)
{
	return Entries.FindByPredicate(
		[&ItemRowName](const FInventoryEntry& Entry)
		{
			return Entry.Item && Entry.Item->GetItemRowHandle().RowName == ItemRowName;
		});
}

const FInventoryEntry* FInventoryList::FindEntry(FName ItemRowName) const
{
	return Entries.FindByPredicate(
		[&ItemRowName](const FInventoryEntry& Entry)
		{
			return Entry.Item && Entry.Item->GetItemRowHandle().RowName == ItemRowName;
		});
}

FInventoryEntry& FInventoryList::AddEntry(UItemInstance* Item)
{
	FInventoryEntry& Entry = Entries.Emplace_GetRef(Item);

	RebuildIndexMap();

	MarkItemDirty(Entry);

	return Entry;
}

bool FInventoryList::RemoveEntry(const FGuid& Guid)
{
	const int32* Index = GuidIndexMap.Find(Guid);
	if (!Index)
	{
		return false;
	}

	Entries.RemoveAt(*Index);

	RebuildIndexMap();

	MarkArrayDirty();

	return true;
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// 이벤트 기반으로만 작동할 것
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryList.Owner = this;

	OnInventoryChangedDelegate.AddUObject(this, &ThisClass::CalculateItemModifiedStats);
	CalculateItemModifiedStats();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventoryList);
}

UAbilitySystemComponent* UInventoryComponent::GetASC() const
{
	const auto* AscOwner = Cast<IASCProvider>(GetOwner());
	if (!AscOwner)
	{
		return nullptr;
	}

	return AscOwner->GetASC();
}

UItemInstance* UInventoryComponent::CreateItemInstance(const FDataTableRowHandle& RowHandle, int32 Count)
{
	check(Count > 0);

	auto* NewItem = NewObject<UItemInstance>(this);
	check(NewItem);

	NewItem->Initialize(this, FGuid::NewGuid(), RowHandle, Count);

	return NewItem;
}

bool UInventoryComponent::CanAddItem(const FDataTableRowHandle& RowHandle, int32 Count) const
{
	return true;
}

bool UInventoryComponent::CanRemoveItem(const FDataTableRowHandle& RowHandle, int32 Count) const
{
	const UItemInstance* Item = FindItem(RowHandle);

	return Item && Item->GetItemCount() >= Count;
}

UItemInstance* UInventoryComponent::AddItem(FName ItemRowName, int32 Count)
{
	check(GetOwner()->HasAuthority());

	if (const UGameInstance* GI = GetOwner()->GetGameInstance())
	{
		if (const auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>())
		{
			if (const UDataTable* ItemTable = DataSubsystem->GetItemInfoTable())
			{
				FDataTableRowHandle RowHandle;
				RowHandle.DataTable = ItemTable;
				RowHandle.RowName = ItemRowName;

				return AddItem(RowHandle, Count);
			}
		}
	}

	return nullptr;
}

UItemInstance* UInventoryComponent::AddItem(const FDataTableRowHandle& RowHandle, int32 Count)
{
	check(GetOwner()->HasAuthority());

	if (Count <= 0)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Count must be greater than zero"), *LOG_LINE_INFO);
		return nullptr;
	}

	if (!CanAddItem(RowHandle, Count))
	{
		RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Cannot add item"), *LOG_LINE_INFO);
		return nullptr;
	}

	if (UItemInstance* ExistingItem = FindItem(RowHandle))
	{
		ExistingItem->AddItemCount(Count);

		InventoryList.MarkDirty(ExistingItem);

		OnItemChanged(ExistingItem);

		// 이후 게임플레이 이펙트 + 브로드캐스트

		return ExistingItem;
	}

	UItemInstance* NewItem = CreateItemInstance(RowHandle, Count);
	if (!IsValid(NewItem))
	{
		return nullptr;
	}

	InventoryList.AddEntry(NewItem);

	OnItemAdded(NewItem);

	// 이후 게임플레이 이펙트 + 브로드캐스트

	return NewItem;
}

bool UInventoryComponent::RemoveItem(const FDataTableRowHandle& RowHandle, int32 Count)
{
	check(GetOwner()->HasAuthority());

	if (Count <= 0)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Invalid count"), *LOG_LINE_INFO);
		return false;
	}

	UItemInstance* Item = FindItem(RowHandle);
	if (!Item)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Item not found"), *LOG_LINE_INFO);
		return false;
	}

	if (!CanRemoveItem(RowHandle, Count))
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Not enough item count"), *LOG_LINE_INFO);
		return false;
	}

	const int32 NewCount = Item->RemoveItemCount(Count);
	if (NewCount > 0)
	{
		InventoryList.MarkDirty(Item);

		OnItemChanged(Item);

		return true;
	}

	const bool bRemoved = InventoryList.RemoveEntry(Item->GetItemGUID());
	if (bRemoved)
	{
		OnItemRemoved(Item);
	}

	return bRemoved;
}

UItemInstance* UInventoryComponent::FindItem(const FGuid& ItemGuid)
{
	if (FInventoryEntry* Entry = InventoryList.FindEntry(ItemGuid))
	{
		return Entry->Item;
	}

	return nullptr;
}

const UItemInstance* UInventoryComponent::FindItem(const FGuid& ItemGuid) const
{
	if (const FInventoryEntry* Entry = InventoryList.FindEntry(ItemGuid))
	{
		return Entry->Item;
	}

	return nullptr;
}

UItemInstance* UInventoryComponent::FindItem(const FDataTableRowHandle& RowHandle)
{
	if (FInventoryEntry* Entry = InventoryList.FindEntry(RowHandle))
	{
		return Entry->Item;
	}

	return nullptr;
}

const UItemInstance* UInventoryComponent::FindItem(const FDataTableRowHandle& RowHandle) const
{
	if (const FInventoryEntry* Entry = InventoryList.FindEntry(RowHandle))
	{
		return Entry->Item;
	}

	return nullptr;
}

UItemInstance* UInventoryComponent::FindItem(FName ItemRowName)
{
	if (auto Entry = InventoryList.FindEntry(ItemRowName))
	{
		return Entry->Item;
	}

	return nullptr;
}

const UItemInstance* UInventoryComponent::FindItem(FName ItemRowName) const
{
	if (const FInventoryEntry* Entry = InventoryList.FindEntry(ItemRowName))
	{
		return Entry->Item;
	}

	return nullptr;
}

int32 UInventoryComponent::GetItemCount(const FGuid& ItemGuid) const
{
	if (const UItemInstance* Item = FindItem(ItemGuid))
	{
		return Item->GetItemCount();
	}

	return 0;
}

int32 UInventoryComponent::GetItemCount(const FDataTableRowHandle& RowHandle) const
{
	if (const UItemInstance* Item = FindItem(RowHandle))
	{
		return Item->GetItemCount();
	}

	return 0;
}

void UInventoryComponent::HandleEntryAdded(const FInventoryEntry& Entry)
{
	InventoryList.RebuildIndexMap();

	if (!Entry.Item)
	{
		return;
	}

	OnItemAdded(Entry.Item);
}

void UInventoryComponent::HandleEntryChanged(const FInventoryEntry& Entry)
{
	InventoryList.RebuildIndexMap();

	if (!Entry.Item)
	{
		return;
	}

	OnItemChanged(Entry.Item);
}

void UInventoryComponent::HandleEntryRemoved(const FInventoryEntry& Entry)
{
	InventoryList.RebuildIndexMap();

	if (!Entry.Item)
	{
		return;
	}

	OnItemRemoved(Entry.Item);
}

TArray<FItemStorageInfo> UInventoryComponent::GetNewAchivedItems() const
{
	TArray<FItemStorageInfo> Result;
	const auto* PS = Cast<APlayerState>(GetOwner());
	if (!IsValid(PS))
	{
		return Result;
	}

	const UGameInstance* GI = GetOwner()->GetGameInstance();
	const auto* DataStorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>();
	if (!DataStorageSubsystem)
	{
		return Result;
	}

	TArray<FItemStorageInfo> PrePreviousItems;
	if (!DataStorageSubsystem->TryGetPrePreviousInventoryStorage(PS, PrePreviousItems))
	{
		return Result;
	}

	for (const FInventoryEntry& Entry : GetEntries())
	{
		if (UItemInstance* Item = Entry.Item)
		{
			if (FItemStorageInfo* Found = PrePreviousItems.FindByPredicate(
				[&Item](const FItemStorageInfo& Prev)
				{
					return !Prev.RowName.IsNone() && Item && Item->GetItemRowHandle().RowName == Prev.RowName;
				}))
			{
				int32 AchiveCount = Item->GetItemCount() - Found->Count;
				if (AchiveCount > 0)
				{
					Result.Emplace(Found->RowName, AchiveCount);
				}
			}
			else
			{
				if (Item->GetItemCount() > 0)
				{
					Result.Emplace(Item->GetItemRowHandle().RowName, Item->GetItemCount());
				}
			}
		}
	}

	return Result;
}

void UInventoryComponent::ApplyGameplayEffect(UItemInstance* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	ensureMsgf(Item->GetItemCount() > 0, TEXT("Item count must be greater than zero"));

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	const FItemTableInfo* ItemInfo = RSUtils::GetTableInfo<FItemTableInfo>(
		Item->GetItemRowHandle(), TEXT("ApplyGameplayEffect"));
	if (!ItemInfo)
	{
		return;
	}

	if (!ItemInfo->MasterItemEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(Item);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ItemInfo->MasterItemEffectClass, 1.0f, EffectContext);
	if (!SpecHandle.IsValid())
	{
		return;
	}

	FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
	if (!Spec)
	{
		return;
	}

	for (const auto& [OptionTag, Value, bScaleWithStack] : ItemInfo->ItemOptions)
	{
		if (!OptionTag.IsValid())
		{
			ensureMsgf(false, TEXT("Item option tag is invalid"));
			continue;
		}

		float Magnitude;
		if (bScaleWithStack)
		{
			Magnitude = Value * Item->GetItemCount();
		}
		else
		{
			Magnitude = Value;
		}

		Spec->SetSetByCallerMagnitude(OptionTag, Magnitude);
	}

	const FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec);
	if (!ActiveHandle.IsValid())
	{
		return;
	}

	ensureMsgf(!Item->GetActiveGameplayEffectHandle().IsValid(), TEXT("Item already has active gameplay effect"));

	Item->SetActiveGameplayEffectHandle(ActiveHandle);
}

void UInventoryComponent::UpdateGameplayEffect(UItemInstance* Item)
{
	RemoveGameplayEffect(Item);

	ApplyGameplayEffect(Item);
}

void UInventoryComponent::RemoveGameplayEffect(UItemInstance* Item)
{
	if (!IsValid(Item))
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		return;
	}

	ensureMsgf(Item->GetActiveGameplayEffectHandle().IsValid(), TEXT("Item does not have active gameplay effect"));

	const FActiveGameplayEffectHandle& Handle = Item->GetActiveGameplayEffectHandle();
	if (!Handle.IsValid())
	{
		return;
	}

	if (ASC->RemoveActiveGameplayEffect(Handle))
	{
		Item->ClearActiveGameplayEffectHandle();
	}
}

void UInventoryComponent::OnItemAdded(UItemInstance* Item)
{
	if (GetOwner()->HasAuthority())
	{
		ApplyGameplayEffect(Item);
	}

	OnItemAddedDelegate.Broadcast(Item);
	OnInventoryChangedDelegate.Broadcast();
}

void UInventoryComponent::OnItemChanged(UItemInstance* Item)
{
	if (GetOwner()->HasAuthority())
	{
		UpdateGameplayEffect(Item);
	}

	OnItemChangedDelegate.Broadcast(Item);
	OnInventoryChangedDelegate.Broadcast();
}

void UInventoryComponent::OnItemRemoved(UItemInstance* Item)
{
	if (GetOwner()->HasAuthority())
	{
		RemoveGameplayEffect(Item);
	}

	OnItemRemovedDelegate.Broadcast(Item);
	OnInventoryChangedDelegate.Broadcast();
}

void UInventoryComponent::CalculateItemModifiedStats()
{
	ItemModifiedStats.Empty();

	for (const FInventoryEntry& Entry : InventoryList.Entries)
	{
		if (const UItemInstance* Item = Entry.Item)
		{
			if (const auto* ItemInfo = RSUtils::GetTableInfo<FItemTableInfo>(
				Item->GetItemRowHandle(), TEXT("CalculateItemModifiedStats")))
			{
				for (const auto& [OptionTag, Value, bScaleWithStack] : ItemInfo->ItemOptions)
				{
					const float Magnitude = bScaleWithStack ? (Value * Item->GetItemCount()) : Value;
					ItemModifiedStats.FindOrAdd(OptionTag) += Magnitude;
				}
			}
		}
	}

	// 스탯을 우선순위에 따라 정렬
	ItemModifiedStats.KeySort([](const FGameplayTag& A, const FGameplayTag& B)
	{
		return RSUtils::GetStatPriority(A) < RSUtils::GetStatPriority(B);
	});
}
