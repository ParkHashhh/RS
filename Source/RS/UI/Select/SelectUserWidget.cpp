// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectUserWidget.h"
#include "RS/Core/LogCategory.h"
#include "RS/Player/Character/SelectPawn.h"

USelectUserWidget::USelectUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USelectUserWidget::PickSelectPawn(ASelectPawn* SelectPawn)
{
	if (IsValid(SlotWidget))
	{
		SlotWidget->PickSelectPawn(SelectPawn);

	}
}
