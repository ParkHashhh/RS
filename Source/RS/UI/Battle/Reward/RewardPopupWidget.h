// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "RewardPopupWidget.generated.h"

class UTileView;
class URewardEntryObject;
class UListView;
/**
 *
 */
UCLASS()
class RS_API URewardPopupWidget : public URSUserWidget
{
	GENERATED_BODY()

	static constexpr int32 MaxRewardCount = 3;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<URewardPopupWidget> RewardPopupWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTileView> RewardTileView;

	UPROPERTY()
	TArray<TObjectPtr<URewardEntryObject>> RewardEntryObjects;

public:
	explicit URewardPopupWidget(const FObjectInitializer& ObjectInitializer);

	void Init(const TArray<FName>& RewardItemRowNames);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
