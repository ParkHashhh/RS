// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/RSUserWidget.h"
#include "EndPopUpWidget.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UEndPopUpWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UEndPopUpWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HomeButton;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION()
	void HomeClick();

};
