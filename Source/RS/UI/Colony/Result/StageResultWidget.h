// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Core/RSTypes.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "StageResultWidget.generated.h"

class UStageResultItemSlotObject;
class UInventoryComponent;
class UStageResultStatEntryObject;
class UListView;
class UStageResultStatEntryWidget;
class UInventoryPopupSlotObject;
class UTileView;
class UInventoryPopupSlotWidget;
class UButton;

/**
 *
 */
UCLASS()
class RS_API UStageResultWidget : public URSUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStageResultStatEntryWidget> StatWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> StatListView;

	UPROPERTY()
	TArray<TObjectPtr<UStageResultStatEntryObject>> StatObjects;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventoryPopupSlotWidget> ItemSlotWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTileView> ItemSlotTileView;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY()
	TArray<TObjectPtr<UStageResultItemSlotObject>> ItemSlotObjects;

	bool bAlreadyInitialized = false;

public:
	void Init(UInventoryComponent* InInventoryComponent);


protected:
	virtual void NativeOnInitialized() override;

private:

	UFUNCTION()
	void OnConfirmClicked();
	void OnGameplayReadyChanged(bool bIsGameplayReady);

	void SetStatistics();
	void SetNewAchievedItems();
};
