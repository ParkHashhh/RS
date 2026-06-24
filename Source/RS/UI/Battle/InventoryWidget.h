// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/RSUserWidget.h"
#include "InventoryWidget.generated.h"

class UWrapBox;
class UInventorySlotWidget;
class UInventoryComponent;
/**
 *
 */
UCLASS()
class RS_API UInventoryWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UInventoryWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UInventoryComponent> CachedInventoryComponent;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> SlotContainer;

	TMap<FName, TObjectPtr<UInventorySlotWidget>> InventorySlots;

	UPROPERTY(EditDefaultsOnly, Category="Inventory", meta=(AllowPrivateAccess=true))
	TSubclassOf<UInventorySlotWidget> InventorySlotWidgetClass;

	FDelegateHandle OnInventoryItemChangedHandle;

public:
	void Init(UInventoryComponent* InventoryComponent);

	FORCEINLINE UInventoryComponent* GetCachedInventoryComponent() const
	{
		return CachedInventoryComponent.Get();
	}

protected:
	virtual void NativeDestruct() override;

private:
	void OnInventoryItemChanged(FName ItemRowName, int32 ItemCount) const;

	void InnerInit();
};
