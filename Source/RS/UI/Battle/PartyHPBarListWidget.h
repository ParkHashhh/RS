// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/RSUserWidget.h"
#include "PartyHPBarListWidget.generated.h"

class UPartyHPBarWidget;
/**
 * 
 */
UCLASS()
class RS_API UPartyHPBarListWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UPartyHPBarListWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPartyHPBarWidget> Row1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPartyHPBarWidget> Row2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPartyHPBarWidget> Row3;

	UPROPERTY()
	TArray<TObjectPtr<UPartyHPBarWidget>> HPBarArray;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void SetPartyList();

};
