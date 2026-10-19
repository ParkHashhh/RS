// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "RSGameUserSettings.generated.h"

/**
 * 
 */
UCLASS()
class RS_API URSGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

protected:
	UPROPERTY(Config)
	float SoundVolume = 1.0f;

	UPROPERTY(Config)
	float MouseSensitivity = 0.7f;

	UPROPERTY(Config)
	float Brightness = 1.0f; // 후처리/감마에 적용할 원시값

	UPROPERTY(Config)
	bool bShowControlsHelp = true;
public:
	FORCEINLINE bool GetShowControlsHelp() const
	{
		return bShowControlsHelp;
	}
	FORCEINLINE float GetSoundVolume() const
	{
		return SoundVolume;
	}
	FORCEINLINE float GetMouseSensitivity() const
	{
		return MouseSensitivity;
	}
	FORCEINLINE float GetBrightness() const
	{
		return Brightness;
	}
	static URSGameUserSettings* Get();

	UFUNCTION()
	void SetSoundVolume(float V);
	UFUNCTION()
	void SetMouseSensitivity(float V);
	UFUNCTION()
	void SetBrightness(float V);

	UFUNCTION()
	void SetShowControlsHelp(bool bValue);
};
