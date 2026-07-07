// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/CharacterBoundUserWidget.h"
#include "AmmoWidget.generated.h"

struct FOnAttributeChangeData;
class UTextBlock;
class UImage;
/**
 *
 */
UCLASS()
class RS_API UAmmoWidget : public UCharacterBoundUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> AmmoBar;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> AmmoBarMaterialInstance;

	float CurrAmmo = 0.0f;
	float MaxAmmo = 1.0f;

protected:
	virtual void NativeOnInitialized() override;

	virtual bool TryBindCharacterDelegates(APlayerCharacterBase* Character) override;

	UFUNCTION(BlueprintImplementableEvent, DisplayName="On Ammo Changed")
	void K2_OnAmmoChanged(float AmmoRatio);

private:
	void OnCurrAmmoChanged(const FOnAttributeChangeData& Data);
	void OnMaxAmmoChanged(const FOnAttributeChangeData& Data);
};
