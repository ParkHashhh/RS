// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RS/Utils/AscProvider.h"
#include "InventoryComponent.generated.h"

class UInventoryComponent;
struct FInventoryArray;
class UDataSubsystem;
struct FItemTableInfo;

USTRUCT(BlueprintType)
struct FInventoryItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemRowName;

	UPROPERTY(BlueprintReadOnly)
	int32 Count{1};

	// 장착 해제 시 효과를 꺼주기 위해 GAS가 발급해 준 핸들을 보관 (로컬/서버 인스턴스 관리용)
	UPROPERTY()
	FActiveGameplayEffectHandle AppliedEffectHandle;

	[[nodiscard]] const FItemTableInfo* GetRowInfo(const UDataSubsystem* Subsystem) const;

	void PreReplicatedRemove(const FInventoryArray& InArraySerializer);
	void PostReplicatedAdd(const FInventoryArray& InArraySerializer);
	void PostReplicatedChange(const FInventoryArray& InArraySerializer);

	void ApplyItemEffect(UInventoryComponent* InventoryComponent);
	void RemoveItemEffect(UInventoryComponent* InventoryComponent);

	friend bool operator==(const FInventoryItem& Lhs, const FInventoryItem& RHS)
	{
		return Lhs.ItemRowName == RHS.ItemRowName
			&& Lhs.AppliedEffectHandle == RHS.AppliedEffectHandle;
	}

	friend bool operator!=(const FInventoryItem& Lhs, const FInventoryItem& RHS)
	{
		return !(Lhs == RHS);
	}
};

USTRUCT(BlueprintType)
struct FInventoryArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItem> Items;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> OwningInventoryComponent{};

	// 네트워크 델타 직렬화 필수 구현
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& InDeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItem, FInventoryArray>(
			Items, InDeltaParms, *this);
	}

	void AddItem(FName InRowName, int32 InCount, const UDataSubsystem* DataSubsystem);
	void RemoveItem(int32 InIndex);
	void MarkItemDirtyAt(int32 InIndex);
};

// NetDeltaSerializer 활성화를 위한 특수화 등록
template <>
struct TStructOpsTypeTraits<FInventoryArray> : public TStructOpsTypeTraitsBase2<FInventoryArray>
{
	enum { WithNetDeltaSerializer = true };
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RS_API UInventoryComponent : public UActorComponent, public IAscProvider
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Inventory", meta=(AllowPrivateAccess=true))
	FInventoryArray InventoryArray;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory|GAS")
	void UseConsumableItem(UPARAM(ref) FInventoryItem& Item);

	// 외부(예: 루팅 시스템 등)에서 인벤토리에 아이템을 지급하기 위한 공용 인터페이스
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void AddItemToInventory(FName InRowName, int32 InCount = 1);

	virtual UAbilitySystemComponent* GetASC() const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE const FInventoryArray& GetInventoryArray() const { return InventoryArray; }
	FORCEINLINE const TArray<FInventoryItem>& GetInventoryItems() const { return InventoryArray.Items; }
};
