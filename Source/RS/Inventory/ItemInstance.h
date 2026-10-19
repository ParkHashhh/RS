// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "UObject/Object.h"
#include "RS/Core/RSTypes.h"
#include "ItemInstance.generated.h"

class UInventoryComponent;
/**
 *
 */
UCLASS()
class RS_API UItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UItemInstance();

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryComponent> OwnerInventory;

	UPROPERTY(Replicated, Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter = K2_GetItemGUID, meta = (AllowPrivateAccess = true))
	FGuid ItemGUID;

	UPROPERTY(ReplicatedUsing = OnRep_ItemRowHandle, Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter = K2_GetItemRowHandle, meta = (AllowPrivateAccess = true))
	FDataTableRowHandle ItemRowHandle;

	UPROPERTY(ReplicatedUsing = OnRep_ItemCount, Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter = K2_GetItemCount, meta = (AllowPrivateAccess = true))
	int32 ItemCount;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle;

public:
	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Initialize(UInventoryComponent* InOwner, const FGuid& InItemGUID, const FDataTableRowHandle& InItemRowHandle, int32 InItemCount);

private:
	// 서브오브젝트 프로퍼티는 소유 배열(FInventoryEntry)과 별개의 타이밍으로 복제되기 때문에,
	// 슬롯 위젯이 먼저 그려진 뒤 뒤늦게 도착해도 UI가 다시 갱신되도록 알려준다.
	UFUNCTION()
	void OnRep_ItemRowHandle();

	UFUNCTION()
	void OnRep_ItemCount();

public:

	FORCEINLINE const FGuid& GetItemGUID() const
	{
		return ItemGUID;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Item GUID")
	const FGuid& K2_GetItemGUID() const
	{
		return GetItemGUID();
	}

	FORCEINLINE const FDataTableRowHandle& GetItemRowHandle() const
	{
		return ItemRowHandle;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Item Row Handle")
	const FDataTableRowHandle& K2_GetItemRowHandle() const
	{
		return GetItemRowHandle();
	}

	const FItemTableInfo* GetItemInfo() const;

	UFUNCTION(BlueprintCallable, DisplayName="Get Item Info")
	FItemTableInfo K2_GetItemInfo() const;

	void SetItemInfo(FName ItemRowName, const UDataTable* DataTable);

	FORCEINLINE int32 GetItemCount() const
	{
		return ItemCount;
	}

	UFUNCTION(BlueprintGetter)
	int32 K2_GetItemCount() const
	{
		return GetItemCount();
	}

	FORCEINLINE int32 AddItemCount(int32 DeltaItemCount)
	{
		check(DeltaItemCount > 0);

		ItemCount += DeltaItemCount;

		return ItemCount;
	}

	FORCEINLINE int32 SetItemCount(int32 NewItemCount)
	{
		ItemCount = NewItemCount;

		return ItemCount;
	}

	FORCEINLINE int32 RemoveItemCount(int32 DeltaItemCount)
	{
		check(DeltaItemCount > 0);

		ItemCount -= DeltaItemCount;

		return ItemCount;
	}

	FORCEINLINE const FActiveGameplayEffectHandle& GetActiveGameplayEffectHandle() const
	{
		return ActiveGameplayEffectHandle;
	}

	FORCEINLINE void SetActiveGameplayEffectHandle(const FActiveGameplayEffectHandle& InHandle)
	{
		ActiveGameplayEffectHandle = InHandle;
	}

	FORCEINLINE void ClearActiveGameplayEffectHandle()
	{
		ActiveGameplayEffectHandle.Invalidate();
	}
};
