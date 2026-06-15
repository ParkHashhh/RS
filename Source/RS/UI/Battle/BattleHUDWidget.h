// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/RSUserWidget.h"
#include "BattleHUDWidget.generated.h"

class UCrosshairWidget;
/**
 *
 */
UCLASS()
class RS_API UBattleHUDWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UBattleHUDWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly, BlueprintGetter=K2_GetCrosshairWidget)
	TObjectPtr<UCrosshairWidget> CrosshairWidget;


public:
	FORCEINLINE UCrosshairWidget* GetCrosshairWidget() const
	{
		return CrosshairWidget;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Crosshair Widget")
	UCrosshairWidget* K2_GetCrosshairWidget() const
	{
		return GetCrosshairWidget();
	}

protected:
	virtual void NativeOnInitialized() override;

private:
	void OnGameplayReadyChanged(bool bGameplayReady) const;
};
