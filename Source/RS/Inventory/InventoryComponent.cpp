// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "RS/Core/LogCategory.h"
#include "RS/Core/RSTypes.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Data/RSDataSettings.h"
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

	const UGameInstance* GI = GetOwner()->GetGameInstance();
	if (!GI)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: GameInstance is nullptr, Row: %s"),
			*LOG_LINE_INFO, *ItemRowName.ToString());
		return nullptr;
	}

	const auto* DataSubsystem = GI->GetSubsystem<UDataSubsystem>();
	if (!DataSubsystem)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: DataSubsystem is nullptr, Row: %s"),
			*LOG_LINE_INFO, *ItemRowName.ToString());
		return nullptr;
	}

	const UDataTable* ItemTable = DataSubsystem->GetItemInfoTable();
	if (!ItemTable)
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: ItemInfoTable is nullptr, Row: %s"),
			*LOG_LINE_INFO, *ItemRowName.ToString());
		return nullptr;
	}

	FDataTableRowHandle RowHandle;
	RowHandle.DataTable = ItemTable;
	RowHandle.RowName = ItemRowName;

	return AddItem(RowHandle, Count);
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

		RS_LOG_DEBUG(LogRSInventory, Warning,
			TEXT("%s: AddItem STACKED, Row: %s, AddedCount: %d, NewTotalCount: %d, GUID: %s"),
			*LOG_LINE_INFO, *RowHandle.RowName.ToString(), Count, ExistingItem->GetItemCount(),
			*ExistingItem->GetItemGUID().ToString());

		// Entry.Item 서브오브젝트가 영영 replicate 안 되는 클라는 개수도 fallback RPC로만 받을 수 있음.
		// 이 RPC를 새 아이템 생성 때만 보내면, 스택 증가분(2개째부터)이 fallback 데이터에 반영이 안 돼서
		// 클라 UI에 계속 최초 개수(1)만 표시되다가 다음 레벨 전환(인벤토리 재구성) 때야 따라잡음.
		if (const int32* EntryIndex = InventoryList.GuidIndexMap.Find(ExistingItem->GetItemGUID()))
		{
			Client_NotifyItemAdded(*EntryIndex, RowHandle.RowName, ExistingItem->GetItemCount());
		}

		// 이후 게임플레이 이펙트 + 브로드캐스트

		return ExistingItem;
	}

	UItemInstance* NewItem = CreateItemInstance(RowHandle, Count);
	if (!IsValid(NewItem))
	{
		return nullptr;
	}

	// 서브오브젝트를 먼저 리플리케이션 등록해야 함 - 배열 엔트리(참조)가 먼저 나가버리면
	// 클라가 "가리키는 대상이 아직 없는" 레퍼런스를 받아서 Entry.Item이 영구적으로 null로 남을 수 있음
	AddReplicatedSubObject(NewItem);
	const int32 NewEntryIndex = InventoryList.Entries.Num();
	InventoryList.AddEntry(NewItem);
	OnItemAdded(NewItem);

	// Entry.Item 서브오브젝트 참조가 리플리케이션에서 영구 유실될 수 있어서,
	// RowName/Count는 순수 데이터로 별도 RPC를 통해 백업 경로로 보냄
	Client_NotifyItemAdded(NewEntryIndex, RowHandle.RowName, Count);

	RS_LOG_DEBUG(LogRSInventory, Warning,
		TEXT("%s: AddItem NEW, Row: %s, Count: %d, GUID: %s, DataTable: %s"),
		*LOG_LINE_INFO, *RowHandle.RowName.ToString(), Count, *NewItem->GetItemGUID().ToString(),
		*GetNameSafe(RowHandle.DataTable));

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
		RemoveReplicatedSubObject(Item);
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

	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: HandleEntryAdded, HasAuthority: %d, Item: %s"),
		*LOG_LINE_INFO, GetOwner()->HasAuthority(), *GetNameSafe(Entry.Item));

	if (!Entry.Item)
	{
		// 배열 엔트리는 왔는데 그 안의 서브오브젝트 참조가 아직 안 풀린 경우.
		// PostReplicatedAdd는 엔트리당 한 번만 불려서 여기서 그냥 포기하면 UI가 영원히 갱신 안 됨 - 재시도해야 함
		const int32 Index = &Entry - InventoryList.Entries.GetData();
		RetryHandleEntryAddedByIndex(Index, 0);
		return;
	}

	OnItemAdded(Entry.Item);
}

void UInventoryComponent::RetryHandleEntryAddedByIndex(int32 EntryIndex, int32 RetryCount)
{
	constexpr int32 MaxRetryCount = 50;
	if (!InventoryList.Entries.IsValidIndex(EntryIndex))
	{
		return;
	}

	UItemInstance* Item = InventoryList.Entries[EntryIndex].Item;
	if (Item)
	{
		RS_LOG_DEBUG(LogRSInventory, Warning,
			TEXT("%s: RetryHandleEntryAddedByIndex resolved after %d retries, Item: %s"),
			*LOG_LINE_INFO, RetryCount, *GetNameSafe(Item));
		OnItemAdded(Item);
		return;
	}

	if (RetryCount >= MaxRetryCount)
	{
		RS_LOG_DEBUG(LogRSInventory, Error,
			TEXT("%s: RetryHandleEntryAddedByIndex GAVE UP after %d retries, EntryIndex: %d - subobject never arrived"),
			*LOG_LINE_INFO, RetryCount, EntryIndex);
		return;
	}

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this,
		[this, EntryIndex, RetryCount]()
		{
			RetryHandleEntryAddedByIndex(EntryIndex, RetryCount + 1);
		}), 0.1f, false);
}

void UInventoryComponent::Client_NotifyItemAdded_Implementation(int32 EntryIndex, FName RowName, int32 Count)
{
	RS_LOG_DEBUG(LogRSInventory, Log, TEXT("%s: Client_NotifyItemAdded, EntryIndex: %d, Row: %s, Count: %d"),
		*LOG_LINE_INFO, EntryIndex, *RowName.ToString(), Count);

	FItemStorageInfo Info;
	Info.RowName = RowName;
	Info.Count = Count;
	FallbackItemDataByIndex.Add(EntryIndex, Info);
	OnInventoryChangedDelegate.Broadcast();
}

void UInventoryComponent::HandleEntryChanged(const FInventoryEntry& Entry)
{
	InventoryList.RebuildIndexMap();

	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: HandleEntryChanged, HasAuthority: %d, Item: %s"),
		*LOG_LINE_INFO, GetOwner()->HasAuthority(), *GetNameSafe(Entry.Item));

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
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: TryGetPrePreviousInventoryStorage FAILED"), *LOG_LINE_INFO);
		return Result;
	}

	RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: PrePreviousItems.Num(): %d, GetEntries().Num(): %d"),
		*LOG_LINE_INFO, PrePreviousItems.Num(), GetEntries().Num());

	int32 EntryIdx = 0;
	for (const FInventoryEntry& Entry : GetEntries())
	{
		FName RowName;
		int32 CurrentCount = 0;

		if (const UItemInstance* Item = Entry.Item)
		{
			RowName = Item->GetItemRowHandle().RowName;
			CurrentCount = Item->GetItemCount();
		}
		else
		{
			// Entry.Item 서브오브젝트 참조가 영구 유실된 경우, RPC로 받아둔 RowName/Count 백업 데이터로 대체
			FItemStorageInfo FallbackInfo;
			if (GetFallbackItemData(EntryIdx, FallbackInfo))
			{
				RowName = FallbackInfo.RowName;
				CurrentCount = FallbackInfo.Count;
			}
		}

		++EntryIdx;

		if (RowName.IsNone() || CurrentCount <= 0)
		{
			continue;
		}

		if (FItemStorageInfo* Found = PrePreviousItems.FindByPredicate(
			[&RowName](const FItemStorageInfo& Prev)
			{
				return !Prev.RowName.IsNone() && Prev.RowName == RowName;
			}))
		{
			int32 AchiveCount = CurrentCount - Found->Count;
			if (AchiveCount > 0)
			{
				Result.Emplace(Found->RowName, AchiveCount);
			}
		}
		else
		{
			Result.Emplace(RowName, CurrentCount);
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
		// 레벨 전환 직후처럼 폰/ASC가 아직 스폰 전일 수 있음(리모트 클라는 RestartPlayer가 RPC 왕복 후에 옴) - 다음 틱에 재시도
		RS_LOG_DEBUG(LogRSInventory, Warning, TEXT("%s: ASC not ready yet, retry next tick, Item: %s"),
			*LOG_LINE_INFO, *GetNameSafe(Item));

		TWeakObjectPtr<UItemInstance> WeakItem(Item);
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this,
			[this, WeakItem]()
			{
				if (WeakItem.IsValid())
				{
					ApplyGameplayEffect(WeakItem.Get());
				}
			}));
		return;
	}

	const FItemTableInfo* ItemInfo = RSUtils::GetTableInfo<FItemTableInfo>(
		Item->GetItemRowHandle(), TEXT("ApplyGameplayEffect"));
	if (!ItemInfo)
	{
		return;
	}

	const URSDataSettings* Settings = GetDefault<URSDataSettings>();
	const TSubclassOf<UGameplayEffect> MasterItemEffectClass = Settings
		                                                            ? Settings->MasterItemEffectClass.LoadSynchronous()
		                                                            : nullptr;
	if (!MasterItemEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(Item);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(MasterItemEffectClass, 1.0f, EffectContext);
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
