// Fill out your copyright notice in the Description page of Project Settings.


#include "IconWidgetBase.h"

#include "Components/Border.h"
#include "Components/Image.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/UI/SlotWidget.h"

UIconWidgetBase::UIconWidgetBase(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void UIconWidgetBase::SetIconTexture(UTexture2D* NewTexture)
{
	if (!IsValid(IconMID) || !IsValid(NewTexture))
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: Invalid texture or material for icon widget"), *LOG_LINE_INFO);
		return;
	}

	IconMID->SetTextureParameterValue(IconTextureParameterName, NewTexture);

	IconTexture = NewTexture;
}

void UIconWidgetBase::SetOutlineColor(const FLinearColor& Color)
{
	if (!IsValid(IconMID))
	{
		return;
	}

	IconMID->SetVectorParameterValue(OutlineColorParameterName, Color);
}

void UIconWidgetBase::SetOutlineThickness(float Thickness)
{
	if (!IsValid(IconMID))
	{
		return;
	}

	IconMID->SetScalarParameterValue(OutlineThicknessParameterName, Thickness);
}

void UIconWidgetBase::SetGrade(EGrade Grade)
{
	const UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);

	const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	const FMiscTableInfo* MiscData = DataSubsystem->GetMiscTableInfo();
	if (!MiscData)
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: Failed to get misc table info"), *LOG_LINE_INFO);
		return;
	}

	switch (Grade)
	{
	case EGrade::Common:
		SetOutlineColor(MiscData->CommonColor);
		SetOutlineThickness(MiscData->CommonIconThickness);
		break;
	case EGrade::Rare:
		SetOutlineColor(MiscData->RareColor);
		SetOutlineThickness(MiscData->RareIconThickness);
		break;
	case EGrade::Epic:
		SetOutlineColor(MiscData->EpicColor);
		SetOutlineThickness(MiscData->EpicIconThickness);
		break;
	case EGrade::Legendary:
		SetOutlineColor(MiscData->LegendaryColor);
		SetOutlineThickness(MiscData->LegendaryIconThickness);
		break;
	}
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

	if (!IsValid(IconImage))
	{
		return;
	}

	IconMID = IconImage->GetDynamicMaterial();
}
