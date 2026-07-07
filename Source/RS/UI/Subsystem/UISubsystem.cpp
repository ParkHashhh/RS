// Fill out your copyright notice in the Description page of Project Settings.


#include "UISubsystem.h"
#include "RS/UI/RSUserWidget.h"

void UUISubsystem::AddWidget(const FName WidgetName, URSUserWidget* Widget)
{
	if (UserWidgets.Contains(WidgetName) || WidgetName.IsNone())
	{
		return;
	}

	UserWidgets.Add(WidgetName, TWeakObjectPtr(Widget));
}

void UUISubsystem::RemoveWidget(const FName WidgetName)
{
	if (!UserWidgets.Contains(WidgetName) || WidgetName.IsNone())
	{
		return;
	}

	UserWidgets.Remove(WidgetName);
}

bool UUISubsystem::Contains(const FName WidgetName) const
{
	return UserWidgets.Contains(WidgetName);
}

URSUserWidget* UUISubsystem::FindWidget(const FName WidgetName) const
{
	return UserWidgets.FindRef(WidgetName).Get();
}

int32 UUISubsystem::FindCombatWidget(TArray<TObjectPtr<URSUserWidget>> OutCombatWidgets) const
{
	OutCombatWidgets.Empty();

	for (auto& [Name, Widget] : UserWidgets)
	{
		if (Widget.IsValid() && Widget.Get()->IsCombatWidget())
		{
			OutCombatWidgets.Add(Widget.Get());
		}
	}

	return OutCombatWidgets.Num();
}
