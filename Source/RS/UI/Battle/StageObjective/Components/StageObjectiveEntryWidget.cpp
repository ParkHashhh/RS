// Fill out your copyright notice in the Description page of Project Settings.


#include "StageObjectiveEntryWidget.h"

#include "StageObjectiveEntryObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "RS/LogCategory.h"
#include "RS/RSConstants.h"

void UStageObjectiveEntryWidget::SetProgressText(const FText& InProgressText) const
{
	ProgressText->SetText(InProgressText);
}

void UStageObjectiveEntryWidget::SetProgressRatio(float ProgressRatio) const
{
	ProgressMaterial->SetScalarParameterValue(ProgressParamName, ProgressRatio);
}

void UStageObjectiveEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ProgressBar)
	{
		ProgressMaterial = ProgressBar->GetDynamicMaterial();
	}
}

void UStageObjectiveEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	EntryObject = Cast<UStageObjectiveEntryObject>(ListItemObject);
	if (!EntryObject)
	{
		return;
	}

	const FText InProgressText = FText::Format(FText::FromString(TEXT("{0} / {1}")),
	                                           FMath::RoundToInt32(EntryObject->Current),
	                                           FMath::RoundToInt32(EntryObject->Goal));

	const float InProgressRatio = FMath::IsNearlyZero(EntryObject->Goal)
		                              ? 0.f
		                              : EntryObject->Current / EntryObject->Goal;

	Init(EntryObject->Title, EntryObject->Description, InProgressText, InProgressRatio);
}

void UStageObjectiveEntryWidget::Init(const FText& Title, const FText& Description, const FText& InProgressText,
                                      float ProgressRatio) const
{
	if (TitleText)
	{
		TitleText->SetText(Title);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(Description);
	}

	if (ProgressText)
	{
		ProgressText->SetText(InProgressText);
	}

	if (ProgressMaterial)
	{
		ProgressMaterial->SetScalarParameterValue(ProgressParamName, ProgressRatio);
	}
}

void UStageObjectiveEntryWidget::OnObjectiveProgressChanged() const
{
	SetProgressRatio(!EntryObject || FMath::IsNearlyZero(EntryObject->Goal)
		                 ? 0.f
		                 : EntryObject->Current / EntryObject->Goal);
}
