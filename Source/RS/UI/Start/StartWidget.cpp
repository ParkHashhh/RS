// Fill out your copyright notice in the Description page of Project Settings.


#include "StartWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RS/RSUtils.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/UI/Subsystem/UISubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "RS/Player/StartPlayerController.h"

UStartWidget::UStartWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UStartWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	StartButton->OnClicked.AddDynamic(this, &UStartWidget::StartClick);
	ExitButton->OnClicked.AddDynamic(this, &UStartWidget::ExitClick);
	OptionButton->OnClicked.AddDynamic(this, &UStartWidget::OptionClick);
}

void UStartWidget::StartClick()
{
	auto* GI = GetGameInstance<URSGameInstance>();
	if (!GI)
	{
		return;
	}

	bool bResumed = false;
	if (auto* PC = Cast<AStartPlayerController>(GetOwningPlayer()))
	{
		bResumed = PC->TryStartSinglePlayer();
	}

	if (bResumed)
	{
		GI->LevelLoading(TEXT("Colony"), TEXT("/Game/Levels/Colony"), TEXT(""));
	}
	else
	{
		GI->LevelLoading(TEXT("Select"), TEXT("/Game/Levels/Select"), TEXT(""));
	}
}
void UStartWidget::ExitClick()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
void UStartWidget::OptionClick()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!LP)
	{
		return;
	}

	auto* UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (!UISubsystem)
	{
		return;
	}

	if (URSUserWidget* SettingsWidget = UISubsystem->FindWidget(FName(TEXT("settings"))))
	{
		SettingsWidget->SetVisibility(ESlateVisibility::Visible);
	}
}
