// Fill out your copyright notice in the Description page of Project Settings.


#include "RSUserWidget.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "RS/LogCategory.h"
#include "Subsystem/UISubsystem.h"

URSUserWidget::URSUserWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void URSUserWidget::PlayWidgetAnimation(FName AnimationName, float StartAtTime, float PlaybackSpeed, bool bForward,
                                        bool bRestoreState, int32 NumberOfLoops)
{
	const TWeakObjectPtr<UWidgetAnimation> InAnimation = Animations.FindRef(AnimationName);
	if (InAnimation.IsValid())
	{
		const auto PlayMode = bForward ? EUMGSequencePlayMode::Forward : EUMGSequencePlayMode::Reverse;
		PlayAnimation(InAnimation.Get(), StartAtTime, NumberOfLoops, PlayMode, PlaybackSpeed, bRestoreState);
	}
}

void URSUserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (GetWidgetName().IsNone())
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: Widget name is none"), *LOG_LINE_INFO)
		return;
	}

	ULocalPlayer* LP = GetOwningLocalPlayer();
	check(LP);

	auto* UiSubsystem = LP->GetSubsystem<UUISubsystem>();
	check(UiSubsystem);

	UiSubsystem->AddWidget(GetWidgetName(), this);

	if (UWidgetBlueprintGeneratedClass* GeneratedClass = GetWidgetTreeOwningClass())
	{
		Animations.Reserve(GeneratedClass->Animations.Num());
		for (UWidgetAnimation* Animation : GeneratedClass->Animations)
		{
			if (IsValid(Animation))
			{
				FString Name = Animation->GetName();
				Name.ReplaceInline(TEXT("_INST"), TEXT(""), ESearchCase::CaseSensitive);

				Animations.Emplace(FName(*Name), Animation);
			}
		}
	}
}

void URSUserWidget::NativeDestruct()
{
	if (GetWidgetName().IsNone())
	{
		RS_LOG_DEBUG(LogRSProperties, Error, TEXT("%s: Widget name is none"), *LOG_LINE_INFO)
		return;
	}

	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!LP)
	{
		return;
	}

	auto* UiSubsystem = LP->GetSubsystem<UUISubsystem>();
	if (!UiSubsystem)
	{
		return;
	}

	UiSubsystem->RemoveWidget(GetWidgetName());

	Super::NativeDestruct();
}
