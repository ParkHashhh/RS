// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/UI/Core/IconWidgetBase.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "RS/Core/LogCategory.h"
#include "RS/Core/RSUtils.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/UI/Core/SlotWidget.h"

UIconWidgetBase::UIconWidgetBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UIconWidgetBase::SetIconSprite(UPaperSprite* NewSprite)
{
	if (!IsValid(NewSprite))
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Invalid sprite for icon widget"), *LOG_LINE_INFO);
		return;
	}

	if (!IsValid(IconImage))
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: IconImage is not ready"), *LOG_LINE_INFO);
		return;
	}

	IconSprite = NewSprite;
	IconImage->SetBrushFromAtlasInterface(IconSprite, true);
}

void UIconWidgetBase::SetIconTexture(UTexture2D* NewTexture)
{
	if (!IsValid(NewTexture))
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: Invalid texture for icon widget"), *LOG_LINE_INFO);
		return;
	}

	if (!IsValid(IconImage))
	{
		RS_LOG_DEBUG(LogRSInventory, Error, TEXT("%s: IconImage is not ready"), *LOG_LINE_INFO);
		return;
	}

	IconTexture = NewTexture;
	IconImage->SetBrushFromTexture(IconTexture);
}

void UIconWidgetBase::SetGradeBorderColor(EGrade Grade)
{
	if (!IsValid(BorderImage))
	{
		return;
	}

	const UGameInstance* GI = GetGameInstance();
	const UDataSubsystem* DataSubsystem = GI ? GI->GetSubsystem<UDataSubsystem>() : nullptr;
	BorderImage->SetColorAndOpacity(RSUtils::GetGradeColor(Grade, DataSubsystem));
}

void UIconWidgetBase::SetBorderVisible(bool bVisible)
{
	if (!IsValid(BorderImage))
	{
		return;
	}

	BorderImage->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

UPaperSprite* UIconWidgetBase::GetIconSprite() const
{
	return IconSprite;
}

UTexture2D* UIconWidgetBase::GetIconTexture() const
{
	return IconTexture;
}

USlotWidget* UIconWidgetBase::GetParentSlotWidget() const
{
	return ParentSlotWidget.Get();
}

void UIconWidgetBase::SetParentSlotWidget(USlotWidget* Parent)
{
	this->ParentSlotWidget = Parent;
}

void UIconWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}
