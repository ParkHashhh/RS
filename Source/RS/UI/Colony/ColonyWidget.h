// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../RSUserWidget.h"
#include "ColonyWidget.generated.h"

/**
 * 
 */
UCLASS()
class RS_API UColonyWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UColonyWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StageButton;

protected:
	virtual void NativeOnInitialized() override;

public:
	UFUNCTION()
	void StageClick();
	
};
