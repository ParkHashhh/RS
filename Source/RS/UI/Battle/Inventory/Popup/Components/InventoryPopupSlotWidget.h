// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "RS/UI/IconWidgetBase.h"
#include "InventoryPopupSlotWidget.generated.h"

class UStageResultItemSlotObject;
class UTextBlock;
/**
 *
 */
UCLASS()
class RS_API UInventoryPopupSlotWidget : public UIconWidgetBase, public IUserObjectListEntry
{
	GENERATED_BODY()

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> BackgroundImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemCountText;

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

private:
	void SetItemCountText(int32 Count) const;

	void SetResultItemSlot(const UStageResultItemSlotObject* ResultSlotObj);
};
