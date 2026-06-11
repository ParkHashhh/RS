// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UITypes.h"
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

	inline static const FName IconTextureParameterName = TEXT("IconTexture");
	inline static const FName OutlineColorParameterName = TEXT("OutlineColor");
	inline static const FName OutlineThicknessParameterName = TEXT("OutlineThickness");

public:
	UIconWidgetBase(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> IconMID;

	UPROPERTY()
	TObjectPtr<UTexture2D> IconTexture;

	TWeakObjectPtr<USlotWidget> ParentSlotWidget;

public:
	UFUNCTION(BlueprintCallable)
	void SetIconTexture(UTexture2D* NewTexture);

	UFUNCTION(BlueprintCallable)
	void SetOutlineColor(const FLinearColor& Color);

	UFUNCTION(BlueprintCallable)
	void SetOutlineThickness(float Thickness);

	UFUNCTION(BlueprintCallable)
	void SetGrade(EGrade Grade);

	FORCEINLINE UTexture2D* GetIconTexture() const;

	FORCEINLINE USlotWidget* GetParentSlotWidget() const;

	FORCEINLINE void SetParentSlotWidget(USlotWidget* Parent);

protected:
	virtual void NativeOnInitialized() override;
};
