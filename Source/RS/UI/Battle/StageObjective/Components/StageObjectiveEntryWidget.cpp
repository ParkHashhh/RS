// Fill out your copyright notice in the Description page of Project Settings.


#include "StageObjectiveEntryWidget.h"

#include "StageObjectiveEntryObject.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

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

	auto* EntryObject = Cast<UStageObjectiveEntryObject>(ListItemObject);
	if (!EntryObject)
	{
		return;
	}
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
		ProgressMaterial->SetScalarParameterValue(FName(TEXT("Progress")), ProgressRatio);
	}
}
