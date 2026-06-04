// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "RS/LogCategory.h"
#include "RS/RSTypes.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/RSPlayerState.h"


const FItemTableInfo* FInventoryItem::GetRowInfo(const UDataSubsystem* Subsystem) const
{
	if (!IsValid(Subsystem) || ItemRowName.IsNone())
	{
		return nullptr;
	}

	return Subsystem->GetItemTableInfo(ItemRowName);
}

void FInventoryItem::PreReplicatedRemove(const FInventoryArray& InArraySerializer)
{
	if (IsValid(InArraySerializer.OwningInventoryComponent)
		&& IsValid(InArraySerializer.OwningInventoryComponent->GetOwner())
		&& InArraySerializer.OwningInventoryComponent->GetOwner()->HasAuthority())
	{
		RemoveItemEffect(InArraySerializer.OwningInventoryComponent);
	}
}

void FInventoryItem::PostReplicatedAdd(const FInventoryArray& InArraySerializer)
{
	if (IsValid(InArraySerializer.OwningInventoryComponent)
		&& IsValid(InArraySerializer.OwningInventoryComponent->GetOwner())
		&& InArraySerializer.OwningInventoryComponent->GetOwner()->HasAuthority())
	{
		ApplyItemEffect(InArraySerializer.OwningInventoryComponent);
	}
}

void FInventoryItem::PostReplicatedChange(const FInventoryArray& InArraySerializer)
{
	if (IsValid(InArraySerializer.OwningInventoryComponent)
		&& IsValid(InArraySerializer.OwningInventoryComponent->GetOwner())
		&& InArraySerializer.OwningInventoryComponent->GetOwner()->HasAuthority())
	{
		RemoveItemEffect(InArraySerializer.OwningInventoryComponent);
		ApplyItemEffect(InArraySerializer.OwningInventoryComponent);
	}
}

void FInventoryItem::ApplyItemEffect(UInventoryComponent* InventoryComponent)
{
	if (!IsValid(InventoryComponent))
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Invalid inventory component"), *LOG_LINE_INFO);
		return;
	}

	auto* ASC = InventoryComponent->GetASC();
	if (!IsValid(ASC))
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Invalid ability system component"), *LOG_LINE_INFO);
		return;
	}

	const auto* World = InventoryComponent->GetWorld();
	check(World);

	const UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);

	const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	const FItemTableInfo* ItemTableInfo = GetRowInfo(DataSubsystem);
	if (!ItemTableInfo || !IsValid(ItemTableInfo->MasterItemEffectClass))
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Invalid item table info or missing effect class for item %s"),
		             *LOG_LINE_INFO, *ItemRowName.ToString());
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(InventoryComponent->GetOwner());

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
		ItemTableInfo->MasterItemEffectClass, 1.f, EffectContext);
	if (SpecHandle.IsValid())
	{
		for (const auto& [OptionTag, Value] : ItemTableInfo->ItemOptions)
		{
			if (OptionTag.IsValid())
			{
				// 수량에 따른 효과를 변경하고 싶으면 이쪽을 수정
				const float TotalValue = Value * static_cast<float>(Count);
				SpecHandle.Data->SetSetByCallerMagnitude(OptionTag, TotalValue);
			}
		}

		AppliedEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	}
}

void FInventoryItem::RemoveItemEffect(UInventoryComponent* InventoryComponent)
{
	if (!IsValid(InventoryComponent) || !AppliedEffectHandle.IsValid())
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Invalid inventory component or applied effect handle"),
		             *LOG_LINE_INFO);
		return;
	}

	auto* ASC = InventoryComponent->GetASC();
	if (IsValid(ASC))
	{
		ASC->RemoveActiveGameplayEffect(AppliedEffectHandle);
	}
	AppliedEffectHandle.Invalidate();
}

void FInventoryArray::AddItem(FName InRowName, int32 InCount, const UDataSubsystem* DataSubsystem)
{
	if (InRowName.IsNone() || InCount <= 0 || !IsValid(DataSubsystem))
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Invalid parameters - RowName: %s, Count: %d, DataSubsystem: %s"),
		             *LOG_LINE_INFO, *InRowName.ToString(), InCount,
		             IsValid(DataSubsystem) ? TEXT("Valid") : TEXT("Invalid"));
		return;
	}

	const FItemTableInfo* ItemTableInfo = DataSubsystem->GetItemTableInfo(InRowName);
	if (!ItemTableInfo)
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Item table info not found for row name %s"), *LOG_LINE_INFO,
		             *InRowName.ToString());
		return;
	}

	// 중첩 가능한 아이템
	if (ItemTableInfo->bIsStackable)
	{
		for (FInventoryItem& ExistingItem : Items)
		{
			if (ExistingItem.ItemRowName == InRowName)
			{
				ExistingItem.Count += InCount;

				// 수량 변화를 알림
				MarkItemDirty(ExistingItem);
				return;
			}
		}
	}

	// 중첩불가 or 중첩 가능하지만 기존 보유 아이템 없음
	FInventoryItem& NewItem = Items.AddDefaulted_GetRef();
	NewItem.ItemRowName = InRowName;
	NewItem.Count = InCount;

	// 신규 추가를 알림
	MarkItemDirty(NewItem);
}

void FInventoryArray::RemoveItem(int32 InIndex)
{
	if (Items.IsValidIndex(InIndex))
	{
		Items.RemoveAt(InIndex);

		MarkArrayDirty();
	}
}

void FInventoryArray::MarkItemDirtyAt(int32 InIndex)
{
	if (Items.IsValidIndex(InIndex))
	{
		MarkItemDirty(Items[InIndex]);
	}
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::UseConsumableItem(FInventoryItem& Item)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	auto* ASC = GetASC();
	if (!IsValid(ASC))
	{
		RS_LOG_DEBUG(LogRSItem, Error, TEXT("%s: Need AbilitySystemComponent to consume item"), *LOG_LINE_INFO);
		return;
	}

	const UWorld* World = GetWorld();
	check(World);

	const UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);

	const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	const FItemTableInfo* ItemTableInfo = Item.GetRowInfo(DataSubsystem);
	if (!ItemTableInfo || ItemTableInfo->UseType != EItemUseType::Consumable || !ItemTableInfo->MasterItemEffectClass)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: Invalid item or missing effect class"), *LOG_LINE_INFO);
		return;
	}

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetOwner());

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
		ItemTableInfo->MasterItemEffectClass, 1.f, EffectContext);

	if (SpecHandle.IsValid())
	{
		// 데이터 테이블에 정의된 n개의 옵션을 순회하며 SetByCaller 값 주입
		for (const auto& [OptionTag, Value] : ItemTableInfo->ItemOptions)
		{
			if (OptionTag.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(OptionTag, Value);
			}
		}

		// 소모품은 나중에 수동으로 해제할 일이 없으므로 반환된 Handle을 저장하지 않고 버린다
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
		RS_LOG_DEBUG(LogRSItem, Log, TEXT("%s: Successfully used consumable item %s"), *LOG_LINE_INFO,
		             *Item.ItemRowName.ToString());

		// 수량 차감 및 그에 따른 효과 갱신
		int32 ItemIndex = InventoryArray.Items.Find(Item);
		if (ItemIndex != INDEX_NONE)
		{
			auto& ChangedItem = InventoryArray.Items[ItemIndex];

			// 일단 끄고
			ChangedItem.RemoveItemEffect(this);
			if (--ChangedItem.Count <= 0)
			{
				InventoryArray.RemoveItem(ItemIndex);
			}
			else
			{
				ChangedItem.ApplyItemEffect(this);
				InventoryArray.MarkItemDirtyAt(ItemIndex);
			}
		}
	}
}

void UInventoryComponent::AddItemToInventory(FName InRowName, int32 InCount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const UWorld* World = GetWorld();
	check(World);

	const UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);

	const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	InventoryArray.AddItem(InRowName, InCount, DataSubsystem);
}

UAbilitySystemComponent* UInventoryComponent::GetASC() const
{
	const auto* AscOwner = Cast<IAscProvider>(GetOwner());
	if (!AscOwner)
	{
		return nullptr;
	}

	return AscOwner->GetASC();
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, InventoryArray);
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
