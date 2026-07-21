// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingUserWidget.h"
#include "Components/ProgressBar.h"

void ULoadingUserWidget::SetLoadingProgress(float Percent)
{
	if (LoadingProgressBar)
	{
		LoadingProgressBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}
