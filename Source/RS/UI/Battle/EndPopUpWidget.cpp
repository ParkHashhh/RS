// Fill out your copyright notice in the Description page of Project Settings.


#include "EndPopUpWidget.h"
#include "RS/Core/RSUtils.h"

UEndPopUpWidget::UEndPopUpWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UEndPopUpWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	HomeButton->OnClicked.AddDynamic(this, &UEndPopUpWidget::HomeClick);
}

void UEndPopUpWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UEndPopUpWidget::HomeClick()
{
	RSUtils::ServerTravelWithDataStorage(GetWorld(), TEXT("Colony"), true);
}
