// Fill out your copyright notice in the Description page of Project Settings.

#include "SettingWidget.h"
#include "../RSGameUserSettings.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "Kismet/KismetSystemLibrary.h"

USettingWidget::USettingWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USettingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	URSGameUserSettings* Settings = URSGameUserSettings::Get();
	if (WindowModeCombo)
	{
		WindowModeCombo->ClearOptions();
		WindowModeCombo->AddOption(TEXT("전체화면"));
		WindowModeCombo->AddOption(TEXT("창모드"));
		WindowModeCombo->AddOption(TEXT("테두리없는창모드"));

		switch (Settings->GetFullscreenMode())
		{
		case EWindowMode::Fullscreen: WindowModeCombo->SetSelectedOption(TEXT("전체화면")); break;
		case EWindowMode::Windowed: WindowModeCombo->SetSelectedOption(TEXT("창모드")); break;
		default: WindowModeCombo->SetSelectedOption(TEXT("테두리없는창모드")); break;
		}
		WindowModeCombo->OnSelectionChanged.AddDynamic(this, &USettingWidget::HandleWindowModeChanged);
	}

	if (ResolutionCombo)
	{
		ResolutionCombo->ClearOptions();
		TArray<FIntPoint> Resolutions;
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);

		FString CurrentRes = FString::Printf(TEXT("%d x %d"), Settings->GetScreenResolution().X, Settings->GetScreenResolution().Y);
		bool bFoundCurrent = false;

		for (const FIntPoint& R : Resolutions)
		{
			FString Option = FString::Printf(TEXT("%d x %d"), R.X, R.Y);
			ResolutionCombo->AddOption(Option);
			if (Option == CurrentRes) bFoundCurrent = true;
		}
		ResolutionCombo->SetSelectedOption(bFoundCurrent ? CurrentRes : ResolutionCombo->GetOptionAtIndex(0));
		ResolutionCombo->OnSelectionChanged.AddDynamic(this, &USettingWidget::HandleResolutionChanged);
	}
	if (GraphicsCombo)
	{
		GraphicsCombo->ClearOptions();
		GraphicsCombo->AddOption(TEXT("Low"));
		GraphicsCombo->AddOption(TEXT("Medium"));
		GraphicsCombo->AddOption(TEXT("High"));
		GraphicsCombo->AddOption(TEXT("Epic"));
		GraphicsCombo->SetSelectedIndex(Settings->GetOverallScalabilityLevel());
		GraphicsCombo->OnSelectionChanged.AddDynamic(this, &USettingWidget::HandleGraphicsChanged);
	}

	if (SoundSlider)
	{
		SoundSlider->SetValue(Settings->GetSoundVolume());
		ApplySoundVolume(Settings->GetSoundVolume());
		SoundSlider->OnValueChanged.AddDynamic(this, &USettingWidget::HandleSoundChanged);
	}

	if (MouseSensSlider)
	{
		MouseSensSlider->SetValue(Settings->GetMouseSensitivity());
		MouseSensSlider->OnValueChanged.AddDynamic(this, &USettingWidget::HandleMouseSensChanged);
	}

	if (BrightnessSlider)
	{
		BrightnessSlider->SetValue(Settings->GetBrightness());
		BrightnessSlider->OnValueChanged.AddDynamic(this, &USettingWidget::HandleBrightnessChanged);
	}

	if (SaveButton)
	{
		SaveButton->OnClicked.AddDynamic(this, &USettingWidget::HandleSaveClicked);
	}
}

void USettingWidget::HandleGraphicsChanged(FString SelectedItem, ESelectInfo::Type SelectType)
{
	URSGameUserSettings* Settings = URSGameUserSettings::Get();
	Settings->SetOverallScalabilityLevel(GraphicsCombo->GetSelectedIndex());
	Settings->ApplySettings(false);
}

void USettingWidget::HandleSoundChanged(float Value)
{
	URSGameUserSettings::Get()->SetSoundVolume(Value);
	ApplySoundVolume(Value);
}

void USettingWidget::HandleMouseSensChanged(float Value)
{
	URSGameUserSettings::Get()->SetMouseSensitivity(Value);
	if (APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(GetOwningPlayerPawn()))
	{
		Player->SetMouseSensitivity(Value, Value);
	}
}

void USettingWidget::HandleBrightnessChanged(float Value)
{
	URSGameUserSettings::Get()->SetBrightness(Value);
	if (APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(GetOwningPlayerPawn()))
	{
		Player->SetBrightness(Value);
	}
}

void USettingWidget::HandleSaveClicked()
{
	URSGameUserSettings::Get()->SaveSettings();
	SetVisibility(ESlateVisibility::Collapsed);
}

void USettingWidget::ApplySoundVolume(float Value)
{
	if (MasterSoundMix && MasterSoundClass && GetWorld())
	{
		UGameplayStatics::SetSoundMixClassOverride(GetWorld(), MasterSoundMix, MasterSoundClass, Value, 1.0f, 0.0f, true);
		UGameplayStatics::PushSoundMixModifier(GetWorld(), MasterSoundMix);
	}
}

void USettingWidget::HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectType)
{
	URSGameUserSettings* Settings = URSGameUserSettings::Get();

	if (SelectedItem == TEXT("전체화면"))
	{
		Settings->SetFullscreenMode(EWindowMode::Fullscreen);
	}
	else if (SelectedItem == TEXT("창모드"))
	{
		Settings->SetFullscreenMode(EWindowMode::Windowed);
	}
	else
	{
		Settings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
	}
	Settings->ApplySettings(false);
}

void USettingWidget::HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectType)
{
	FString Left, Right;
	if (SelectedItem.Split(TEXT(" x "), &Left, &Right))
	{
		FIntPoint Resolution(FCString::Atoi(*Left), FCString::Atoi(*Right));
		URSGameUserSettings* Settings = URSGameUserSettings::Get();
		Settings->SetScreenResolution(Resolution);
		Settings->ApplySettings(false);
	}
}
