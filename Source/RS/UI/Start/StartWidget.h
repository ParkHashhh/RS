// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../RSUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "StartWidget.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UStartWidget : public URSUserWidget
{
	GENERATED_BODY()
public:
	UStartWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ExitButton;

protected:
	virtual void NativeOnInitialized() override;

public:
	UFUNCTION()
	void StartClick();

	UFUNCTION()
	void ExitClick();
};
