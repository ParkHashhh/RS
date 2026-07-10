// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionWidget.h"

#include "Components/Image.h"
#include "PaperSprite.h"

UInteractionWidget::UInteractionWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UInteractionWidget::Init(bool bInteractable)
{
	SetInputEnabled(bInteractable);
	SetProgressEnable(false);
}

void UInteractionWidget::SetInputEnabled(bool bEnabled) const
{
	if (InputKeyIcon)
	{
		InputKeyIcon->SetBrushFromAtlasInterface(bEnabled ? InputSpriteEnabled : InputSpriteDisabled);
	}
}

void UInteractionWidget::SetProgress(float Progress) const
{
	if (bIsProgressEnable && ProgressMaterial)
	{
		ProgressMaterial->SetScalarParameterValue(FName(TEXT("Progress")), FMath::Clamp(Progress, 0.0f, 1.0f));
	}
}

void UInteractionWidget::SetProgressEnable(bool bEnable)
{
	if (bIsProgressEnable == bEnable)
	{
		return;
	}

	bIsProgressEnable = bEnable;

	if (ProgressImage)
	{
		ProgressImage->SetVisibility(bEnable ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UInteractionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ProgressImage)
	{
		ProgressMaterial = ProgressImage->GetDynamicMaterial();
	}
}
