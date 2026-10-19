// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/UI/Core/PauseWidget.h"

#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RS/UI/Subsystem/UISubsystem.h"

UPauseWidget::UPauseWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UPauseWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &UPauseWidget::HandleSettingsClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UPauseWidget::HandleQuitClicked);
	}
}

void UPauseWidget::HandleSettingsClicked()
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

void UPauseWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
