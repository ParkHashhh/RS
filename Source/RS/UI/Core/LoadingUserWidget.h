// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "LoadingUserWidget.generated.h"

class UProgressBar;

/**
 * 
 */
UCLASS()
class RS_API ULoadingUserWidget : public URSUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> LoadingProgressBar;

public:
	void SetLoadingProgress(float Percent);
};
