// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../RSUserWidget.h"
#include "Components/Button.h"
#include "MapSelectWidget.generated.h"

/**
 * 
 */

class UMapSelectCardWidget;

UCLASS()
class RS_API UMapSelectWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UMapSelectWidget(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMapSelectCardWidget> Card_Warehouse;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMapSelectCardWidget> Card_SwimmingPool;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UMapSelectCardWidget> Card_Construct;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> QuitButton;

	TObjectPtr<UMapSelectCardWidget> CurrentSelectedCard;
protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleCardClicked(UMapSelectCardWidget* ClickedCard);

	UFUNCTION()
	void HandleConfirmClicked();

	UFUNCTION()
	void HandleQuitClicked();

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
};
