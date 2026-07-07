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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, BlueprintGetter=K2_GetWidgetName, meta=(AllowPrivateAccess = true))
	FName WidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	bool bIsCombatWidget = false;

	TMap<FName, TWeakObjectPtr<UWidgetAnimation>> Animations;

public:
	FORCEINLINE FName GetWidgetName() const
	{
		return WidgetName;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Widget Name")
	FName K2_GetWidgetName() const
	{
		return GetWidgetName();
	}

	void PlayWidgetAnimation(FName AnimationName, float StartAtTime = 0.0f, float PlaybackSpeed = 1.f,
	                         bool bForward = true, bool bRestoreState = false, int32 NumberOfLoops = 1);

	UFUNCTION(BlueprintPure)
	bool IsCombatWidget() const
	{
		return bIsCombatWidget;
	}

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
};
