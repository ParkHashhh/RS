// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UISubsystem.generated.h"

class URSUserWidget;
/**
 *
 */
UCLASS()
class RS_API UUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

private:
	TMap<FName, TWeakObjectPtr<URSUserWidget>> UserWidgets;

public:
	void AddWidget(const FName WidgetName, URSUserWidget* Widget);

	void RemoveWidget(const FName WidgetName);

	bool Contains(const FName WidgetName) const;

	URSUserWidget* GetWidget(const FName WidgetName) const;
};
