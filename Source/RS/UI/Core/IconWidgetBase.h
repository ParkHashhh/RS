// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RS/Core/RSTypes.h"
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

	// 등급 테두리용. WBP에 같은 이름의 Image 위젯이 있어야 연결됨(없어도 컴파일은 됨).
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> BorderImage;

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

	// 아이템 등급(Common/Rare/Epic/Legendary)에 맞는 색으로 테두리 이미지를 칠한다.
	UFUNCTION(BlueprintCallable)
	void SetGradeBorderColor(EGrade Grade);

	// 빈 슬롯일 때 테두리를 숨기기 위함 (풀링되는 리스트 항목이 이전 아이템의 테두리를 물려받는 것 방지)
	UFUNCTION(BlueprintCallable)
	void SetBorderVisible(bool bVisible);

	UImage* GetIconImage() const
	{
		return IconImage;
	}

	UPaperSprite* GetIconSprite() const;

	UTexture2D* GetIconTexture() const;

	USlotWidget* GetParentSlotWidget() const;

	void SetParentSlotWidget(USlotWidget* Parent);

protected:
	virtual void NativeOnInitialized() override;
};
