// Fill out your copyright notice in the Description page of Project Settings.

#include "MapSelectWidget.h"
#include "MapSelectCardWidget.h"
#include "RS/GameMode/RSGameInstance.h"
#include "../../Player/ColonyPlayerController.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"

UMapSelectWidget::UMapSelectWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UMapSelectWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Card_Warehouse)
		Card_Warehouse->OnCardClicked.AddDynamic(this, &UMapSelectWidget::HandleCardClicked);
	if (Card_SwimmingPool)
		Card_SwimmingPool->OnCardClicked.AddDynamic(this, &UMapSelectWidget::HandleCardClicked);
	if (Card_Construct)
		Card_Construct->OnCardClicked.AddDynamic(this, &UMapSelectWidget::HandleCardClicked);

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UMapSelectWidget::HandleConfirmClicked);
		ConfirmButton->SetIsEnabled(false);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UMapSelectWidget::HandleQuitClicked);
	}
}

void UMapSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DifficultyStageText)
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (const auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				DifficultyStageText->SetText(FText::AsNumber(StorageSubsystem->GetCurrentDifficultyStage()));
			}
		}
	}
}

void UMapSelectWidget::HandleCardClicked(UMapSelectCardWidget* ClickedCard)
{
	if (CurrentSelectedCard && CurrentSelectedCard != ClickedCard)
	{
		CurrentSelectedCard->SetSelected(false);
	}

	CurrentSelectedCard = ClickedCard;

	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(true);
	}
}

void UMapSelectWidget::HandleConfirmClicked()
{
	if (!CurrentSelectedCard)
	{
		return;
	}

	if (AColonyPlayerController* PC = Cast<AColonyPlayerController>(GetOwningPlayer()))
	{
		
		PC->Server_RequestPortal(CurrentSelectedCard->GetLevelName(), CurrentSelectedCard->GetLevelPackagePath());
		PC->RemoveMapSelectWidget();
	}
}

void UMapSelectWidget::HandleQuitClicked()
{
	if (AColonyPlayerController* PC = Cast<AColonyPlayerController>(GetOwningPlayer()))
	{
		PC->RemoveMapSelectWidget();
	}
}

FReply UMapSelectWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		HandleQuitClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
