// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TestHUD.generated.h"

/**
 *
 */
UCLASS()
class RS_API ATestHUD : public AHUD
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	FText DebugText;

public:
	virtual void DrawHUD() override;

	void SetDebugText(const FText& NewText);
};
