// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../RSUserWidget.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "SettingWidget.generated.h"
class UComboBoxString;
class USlider;
class UButton;
/**
 * 
 */
UCLASS()
class RS_API USettingWidget : public URSUserWidget
{
	GENERATED_BODY()
public:
	USettingWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> ResolutionCombo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> WindowModeCombo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UComboBoxString> GraphicsCombo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> SoundSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> MouseSensSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USlider> BrightnessSlider;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SaveButton;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundMix> MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundClass> MasterSoundClass;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleGraphicsChanged(FString SelectedItem, ESelectInfo::Type SelectType);

	UFUNCTION()
	void HandleSoundChanged(float Value);

	UFUNCTION()
	void HandleMouseSensChanged(float Value);

	UFUNCTION()
	void HandleBrightnessChanged(float Value);

	UFUNCTION()
	void HandleSaveClicked();

	void ApplySoundVolume(float Value);
	UFUNCTION()
	void HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectType);

	UFUNCTION()
	void HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectType);
};
