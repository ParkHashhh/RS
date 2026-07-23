// Fill out your copyright notice in the Description page of Project Settings.


#include "TestHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"

void ATestHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine && GEngine->GetSmallFont())
	{
		FCanvasTextItem TextItem(FVector2D(100.0f, 100.0f), DebugText, GEngine->GetSmallFont(), FLinearColor::Red);
		Canvas->DrawItem(TextItem);
	}
}

void ATestHUD::SetDebugText(const FText& NewText)
{
	DebugText = NewText;
}
