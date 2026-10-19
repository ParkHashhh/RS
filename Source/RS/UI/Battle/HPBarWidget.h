// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/CharacterBoundUserWidget.h"
#include "HPBarWidget.generated.h"

struct FOnAttributeChangeData;
class UProgressBar;
/**
 *
 */
UCLASS()
class RS_API UHPBarWidget : public UCharacterBoundUserWidget
{
	GENERATED_BODY()

public:
	UHPBarWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly, Category="HPBar|Asset")
	TObjectPtr<UProgressBar> HPBar;

	float CurrHP = 0.0f;
	float MaxHP = 1.0f;

protected:
	virtual void NativeOnInitialized() override;

	virtual bool TryBindCharacterDelegates(APlayerCharacterBase* Character) override;

	UFUNCTION(BlueprintImplementableEvent, DisplayName="On HP Changed", Category="HPBar")
	void K2_OnHPChanged(float HPRatio);

private:
	void OnCurrHpChanged(const FOnAttributeChangeData& Data);
	void OnMaxHpChanged(const FOnAttributeChangeData& Data);
};
