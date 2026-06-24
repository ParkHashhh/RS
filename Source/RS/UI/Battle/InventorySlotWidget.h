// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/IconWidgetBase.h"
#include "InventorySlotWidget.generated.h"

class UTextBlock;

/**
 *
 */
UCLASS()
class RS_API UInventorySlotWidget : public UIconWidgetBase
{
	GENERATED_BODY()

public:
	UInventorySlotWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	FName ItemRowName;

	UPROPERTY(Transient, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	int32 ItemCount;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemCountText;

public:
	void Init(FName NewItemRowName, int32 NewItemCount);

	void SetItemCount(int32 NewItemCount);

protected:
	virtual void NativeOnInitialized() override;

private:
	void InnerSetItem();
	void InnerSetItemCount(int32 NewItemCount);

	void OnItemTableLoaded();
};
