// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/RSUserWidget.h"
#include "InventoryPopupWidget.generated.h"

class UGameplayEffect;
struct FGameplayTag;
class UStatEntryObject;
class UListView;
class UStatEntryWidget;
class UInventoryPopupSlotObject;
class UTileView;
class UInventoryPopupSlotWidget;
class UInventoryComponent;
/**
 *
 */
UCLASS()
class RS_API UInventoryPopupWidget : public URSUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess))
	TSubclassOf<UGameplayEffect> EquipItemEffectClass;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	int32 MaxSlotCount = 24;

	UPROPERTY(Transient)
	TObjectPtr<UInventoryComponent> InventoryComponent;

#pragma region ItemSlot

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryPopupSlotWidget> InventoryPopupSlotWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTileView> InventoryTileView;

	UPROPERTY()
	TArray<TObjectPtr<UInventoryPopupSlotObject>> SlotObjects;

#pragma endregion ItemSlot

#pragma region Stat

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStatEntryWidget> StatDescriptionWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> StatListView;

	UPROPERTY()
	TArray<TObjectPtr<UStatEntryObject>> StatObjects;

#pragma endregion Stat

public:
	void Init(UInventoryComponent* InInventoryComponent);

	void RefreshInventory();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	void RefreshStatList();

	void AddBasicStats();
	void AddItemEffects();

	void AddStat(const FGameplayTag& StatTag, const FText& StatName, const FText& StatDescription);

	void OnGameplayReadyChanged(bool bGameplayReady);

	float GetAttributeValueWithTag(const FGameplayTag& TargetTag) const;
};
