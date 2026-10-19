// Fill out your copyright notice in the Description page of Project Settings.

#include "MapSelectWidget.h"
#include "MapSelectCardWidget.h"
#include "RS/GameFramework/RSGameInstance.h"
#include "RS/Player/Controller/ColonyPlayerController.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "RS/GameFramework/GameState/RSGameState.h"

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
		// UDataStorageSubsystem::CurrentDifficultyStage는 머신별 로컬(GameInstanceSubsystem) 값이라
		// 원격 클라에는 절대 전달되지 않음 - 리플리케이트되는 GameState 프로퍼티를 읽어야 호스트/클라
		// 둘 다 보스를 잡은 뒤 정확한 Level을 본다.
		if (const auto* GS = GetWorld() ? GetWorld()->GetGameState<ARSGameState>() : nullptr)
		{
			DifficultyStageText->SetText(FText::AsNumber(GS->GetDifficultyStage()));
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
