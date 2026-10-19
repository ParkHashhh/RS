// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "RewardEntryWidget.generated.h"

class URewardPopupWidget;
class UTexture2D;
class UStatEntryObject;
class UStatEntryWidget;
class UListView;
class UTextBlock;
class UImage;
/**
 *
 */
UCLASS()
class RS_API URewardEntryWidget : public URSUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<URewardPopupWidget> ParentWidget;

	UPROPERTY(Transient)
	FName ItemRowName;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> NormalFrameTexture;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> HoveringFrameTexture;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> FrameImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> IconBorderImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameTextBlock;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> StatListView;

public:
	URewardEntryWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
};
