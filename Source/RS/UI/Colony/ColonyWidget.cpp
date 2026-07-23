// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyWidget.h"

#include "RS/LogCategory.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/Player/ColonyPlayerController.h"

UColonyWidget::UColonyWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UColonyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	StageButton->OnClicked.AddDynamic(this, &UColonyWidget::StageClick);
}

void UColonyWidget::StageClick()
{
	const auto* PC = GetOwningPlayer<AColonyPlayerController>();
	if (!PC)
	{
		return;
	}

	if (URSGameInstance* GI = GetGameInstance<URSGameInstance>())
	{
		GI->LevelLoading(TEXT("Stage_Warehouse"), TEXT("/Game/Levels/Stage_Warehouse"), TEXT(""), false, true, true);
	}
}
