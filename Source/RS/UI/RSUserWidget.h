// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RSUserWidget.generated.h"

/**
 *
 */
UCLASS()
class RS_API URSUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URSUserWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = true))
	FName WidgetName;

	TMap<FName, TWeakObjectPtr<UWidgetAnimation>> Animations;

public:
	FName GetWidgetName() const
	{
		return WidgetName;
	}

	void PlayWidgetAnimation(FName AnimationName, float StartAtTime = 0.0f, float PlaybackSpeed = 1.f,
	                         bool bForward = true, bool bRestoreState = false, int32 NumberOfLoops = 1);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
};
