// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/CharacterBoundUserWidget.h"
#include "BossHPBarWidget.generated.h"

class UTextBlock;
class UProgressBar;
struct FOnAttributeChangeData;


UCLASS()
class RS_API UBossHPBarWidget : public UCharacterBoundUserWidget
{
	GENERATED_BODY()

public:
	UBossHPBarWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BossName;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true), BlueprintReadOnly, Category = "HPBar|Asset")
	TObjectPtr<UProgressBar> BossHPBar;

	float CurrHP = 0.0f;
	float MaxHP = 1.0f;

protected:
	virtual void NativeOnInitialized() override;

	virtual bool TryBindCharacterDelegates(APlayerCharacterBase* Character) override;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On HP Changed", Category = "HPBar")
	void K2_OnHPChanged(float HPRatio);

private:
	void OnCurrHpChanged(const FOnAttributeChangeData& Data);
	void OnMaxHpChanged(const FOnAttributeChangeData& Data);

	
};
