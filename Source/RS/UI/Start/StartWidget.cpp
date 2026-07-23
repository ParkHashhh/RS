// Fill out your copyright notice in the Description page of Project Settings.


#include "StartWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RS/RSUtils.h"
#include "RS/GameMode/RSGameInstance.h"
#include "Kismet/GameplayStatics.h"


UStartWidget::UStartWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UStartWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	StartButton->OnClicked.AddDynamic(this, &UStartWidget::StartClick);
	ExitButton->OnClicked.AddDynamic(this, &UStartWidget::ExitClick);
}

void UStartWidget::StartClick()
{
	if (auto* GI = GetGameInstance<URSGameInstance>())
	{
		FString Option = "";
		GI->LevelLoading(TEXT("Select"),TEXT("/Game/Levels/Select"), Option);
	}
}

void UStartWidget::ExitClick()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
