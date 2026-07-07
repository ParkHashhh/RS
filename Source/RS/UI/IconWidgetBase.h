// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RS/RSTypes.h"
#include "IconWidgetBase.generated.h"

class UImage;
class USlotWidget;
/**
 *
 */
UCLASS()
class RS_API UIconWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UIconWidgetBase(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY()
	TObjectPtr<UTexture2D> IconTexture;

	UPROPERTY()
	TObjectPtr<UPaperSprite> IconSprite;

	TWeakObjectPtr<USlotWidget> ParentSlotWidget;

public:
	UFUNCTION(BlueprintCallable)
	void SetIconSprite(UPaperSprite* NewSprite);

	UFUNCTION(BlueprintCallable)
	void SetIconTexture(UTexture2D* NewTexture);

	FORCEINLINE UPaperSprite* GetIconSprite() const;

	FORCEINLINE UTexture2D* GetIconTexture() const;

	FORCEINLINE USlotWidget* GetParentSlotWidget() const;

	FORCEINLINE void SetParentSlotWidget(USlotWidget* Parent);

protected:
	virtual void NativeOnInitialized() override;
};
