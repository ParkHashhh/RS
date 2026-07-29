// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/CharacterBoundUserWidget.h"
#include "AmmoWidget.generated.h"

struct FOnAttributeChangeData;
class UTextBlock;
class UProgressBar;
/**
 *
 */
UCLASS()
class RS_API UAmmoWidget : public UCharacterBoundUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UMaterialInterface> CurrentMI;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UMaterialInterface> MI_UseDivisions_True;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UMaterialInterface> MI_UseDivisions_False;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess=true))
	int32 HideDivisionLineCount = 50;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> AmmoBarMaterialInstance;

	float CurrAmmo = 0.0f;
	float MaxAmmo = 1.0f;

protected:
	virtual void NativeOnInitialized() override;

	virtual bool TryBindCharacterDelegates(APlayerCharacterBase* Character) override;

private:
	void OnCurrAmmoChanged(const FOnAttributeChangeData& Data);
	void OnMaxAmmoChanged(const FOnAttributeChangeData& Data);

	void OnAmmoChanged();

	void RefreshWidget();
};
