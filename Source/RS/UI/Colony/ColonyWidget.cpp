// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyWidget.h"
#include "RS/LogCategory.h"
#include "RS/RSUtils.h"
#include "RS/Player/RSPlayerState.h"
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
	AColonyPlayerController* PC = Cast<AColonyPlayerController>(GetOwningPlayer());
	if (!PC)
	{
		return;
	}
	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->LevelLoading(TEXT("Stage_Warehouse"), TEXT("/Game/Levels/Stage_Warehouse"), TEXT(""),false,true);
	}
}
