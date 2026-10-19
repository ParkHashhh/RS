// Fill out your copyright notice in the Description page of Project Settings.


#include "MapSelectCardWidget.h"
#include "RS/Core/LogCategory.h"

UMapSelectCardWidget::UMapSelectCardWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UMapSelectCardWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (HitButton)
	{
		HitButton->OnClicked.AddDynamic(this, &UMapSelectCardWidget::HandleHitAreaClicked);
	}

	SetSelected(false);
}

void UMapSelectCardWidget::HandleHitAreaClicked()
{
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("HandleHitAreaClicked"));

	SetSelected(true);
	OnCardClicked.Broadcast(this);
}

void UMapSelectCardWidget::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;

	if (SelectBorder)
	{
		FLinearColor Color = SelectedTint;
		Color.A = bSelected ? SelectedTint.A : 0.f;
		SelectBorder->SetBrushColor(Color);
	}
}
