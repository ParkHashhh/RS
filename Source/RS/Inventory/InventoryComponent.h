// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "ItemInstance.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RS/Utils/ASCProvider.h"
#include "InventoryComponent.generated.h"

struct FInventoryList;
class UItemInstance;
class UInventoryComponent;
class UDataSubsystem;
struct FItemTableInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnItemChanged, const UItemInstance*);
DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

USTRUCT(BlueprintType)
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UItemInstance> Item{};

	FInventoryEntry() = default;

	explicit FInventoryEntry(UItemInstance* InItem)
		: Item(InItem)
	{
	}

	FORCEINLINE bool IsValid() const
	{
		return ::IsValid(Item);
	}

	void PreReplicatedRemove(const FInventoryList& InArraySerializer);
	void PostReplicatedAdd(const FInventoryList& InArraySerializer);
	void PostReplicatedChange(const FInventoryList& InArraySerializer);

	friend bool operator==(const FInventoryEntry& Lhs, const FInventoryEntry& RHS)
	{
		if (!Lhs.Item || !RHS.Item)
		{
			return false;
		}

		return Lhs.Item->GetItemGUID() == RHS.Item->GetItemGUID();
	}

	friend bool operator!=(const FInventoryEntry& Lhs, const FInventoryEntry& RHS)
	{
		return !(Lhs == RHS);
	}
};

USTRUCT(BlueprintType)
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	friend class UInventoryComponent;

	UPROPERTY()
	TArray<FInventoryEntry> Entries{};

	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryComponent> Owner{};

private:
	UPROPERTY(NotReplicated)
	TMap<FGuid, int32> GuidIndexMap{};

public:
	FORCEINLINE bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FInventoryEntry, FInventoryList>(Entries, DeltaParams, *this);
	}

	void MarkDirty(const UItemInstance* Item);

	FInventoryEntry* FindEntry(const FGuid& Guid);
	const FInventoryEntry* FindEntry(const FGuid& Guid) const;

	FInventoryEntry* FindEntry(const FDataTableRowHandle& RowHandle);
	const FInventoryEntry* FindEntry(const FDataTableRowHandle& RowHandle) const;

	FInventoryEntry* FindEntry(FName ItemRowName);
	const FInventoryEntry* FindEntry(FName ItemRowName) const;

	FInventoryEntry& AddEntry(UItemInstance* Item);

	bool RemoveEntry(const FGuid& Guid);

private:
	void RebuildIndexMap();
};

// NetDeltaSerializer 활성화를 위한 특수화 등록
template <>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class RS_API UInventoryComponent : public UActorComponent
                                   , public IASCProvider
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

private:
	UPROPERTY(Replicated)
	FInventoryList InventoryList;

public:
	FOnItemChanged OnItemAddedDelegate;
	FOnItemChanged OnItemChangedDelegate;
	FOnItemChanged OnItemRemovedDelegate;

	FOnInventoryChanged OnInventoryChangedDelegate;

protected:
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetASC() const override;

#pragma region Inventory

public:
	UItemInstance* CreateItemInstance(const FDataTableRowHandle& RowHandle, int32 Count);

	bool CanAddItem(const FDataTableRowHandle& RowHandle, int32 Count) const;
	bool CanRemoveItem(const FDataTableRowHandle& RowHandle, int32 Count) const;

	UItemInstance* AddItem(FName ItemRowName, int32 Count);
	UItemInstance* AddItem(const FDataTableRowHandle& RowHandle, int32 Count);

	bool RemoveItem(const FDataTableRowHandle& RowHandle, int32 Count);

	UItemInstance* FindItem(const FGuid& ItemGuid);
	const UItemInstance* FindItem(const FGuid& ItemGuid) const;

	UItemInstance* FindItem(const FDataTableRowHandle& RowHandle);
	const UItemInstance* FindItem(const FDataTableRowHandle& RowHandle) const;

	UItemInstance* FindItem(FName ItemRowName);
	const UItemInstance* FindItem(FName ItemRowName) const;

	int32 GetItemCount(const FGuid& ItemGuid) const;
	int32 GetItemCount(const FDataTableRowHandle& RowHandle) const;

	void HandleEntryAdded(const FInventoryEntry& Entry);
	void HandleEntryChanged(const FInventoryEntry& Entry);
	void HandleEntryRemoved(const FInventoryEntry& Entry);

	const TArray<FInventoryEntry>& GetEntries() const
	{
		return InventoryList.Entries;
	}

	const FInventoryEntry* GetEntry(int32 SlotIndex) const
	{
		return InventoryList.Entries.IsValidIndex(SlotIndex) ? &InventoryList.Entries[SlotIndex] : nullptr;
	}

	int32 GetEntryCount() const
	{
		return InventoryList.Entries.Num();
	}

	TArray<FItemStorageInfo> GetNewAchivedItems() const;

protected:
	void ApplyGameplayEffect(UItemInstance* Item);
	void UpdateGameplayEffect(UItemInstance* Item);
	void RemoveGameplayEffect(UItemInstance* Item);

private:
	void OnItemAdded(UItemInstance* Item);
	void OnItemChanged(UItemInstance* Item);
	void OnItemRemoved(UItemInstance* Item);

#pragma endregion
};
