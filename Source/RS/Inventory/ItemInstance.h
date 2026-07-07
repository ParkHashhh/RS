// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "UObject/Object.h"
#include "RS/RSTypes.h"
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

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_GetItemGUID, meta=(AllowPrivateAccess=true))
	FGuid ItemGUID;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_GetItemRowHandle, meta=(AllowPrivateAccess=true))
	FDataTableRowHandle ItemRowHandle;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_GetItemCount, meta=(AllowPrivateAccess=true))
	int32 ItemCount;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle ActiveGameplayEffectHandle;

public:
	void Initialize(UInventoryComponent* InOwner, const FGuid& InItemGUID, const FDataTableRowHandle& InItemRowHandle, int32 InItemCount);

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
