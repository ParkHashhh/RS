// Fill out your copyright notice in the Description page of Project Settings.


#include "RSGameUserSettings.h"
#include "Engine/Engine.h"

URSGameUserSettings* URSGameUserSettings::Get()
{
	return CastChecked<URSGameUserSettings>(GEngine->GetGameUserSettings());
}

void URSGameUserSettings::SetSoundVolume(float Value)
{
	SoundVolume = FMath::Clamp(Value, 0.f, 1.f);
}

void URSGameUserSettings::SetMouseSensitivity(float Value)
{
	MouseSensitivity = FMath::Clamp(Value, 0.1f, 5.f);
}

void URSGameUserSettings::SetBrightness(float Value)
{
	Brightness = FMath::Clamp(Value, 0.f, 2.f);
}

void URSGameUserSettings::SetShowControlsHelp(bool bValue)
{
	bShowControlsHelp = bValue;
}
