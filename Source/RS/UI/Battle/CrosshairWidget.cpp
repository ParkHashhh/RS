// Fill out your copyright notice in the Description page of Project Settings.


#include "CrosshairWidget.h"

#include "RS/Player/RSPlayerController.h"

UCrosshairWidget::UCrosshairWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

ARSPlayerController* UCrosshairWidget::GetCachedPlayerController()
{
	if (!IsValid(CachedPlayerController))
	{
		CachedPlayerController = GetOwningPlayer<ARSPlayerController>();
	}

	return CachedPlayerController;
}

bool UCrosshairWidget::TryCachePlayerController()
{
	if (IsValid(CachedPlayerController))
	{
		return true;
	}

	CachedPlayerController = GetOwningPlayer<ARSPlayerController>();
	if (IsValid(CachedPlayerController))
	{
		if (GEngine && WaitPlayerControllerHandle.IsValid())
		{
			// GEngine->OnLocalPlayerControllerReady.Remove(WaitPlayerControllerHandle);
		}
		return true;
	}

	return false;
}

void UCrosshairWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCrosshairWidget::OnPlayerControllerReady(ARSPlayerController* PlayerController)
{
}
