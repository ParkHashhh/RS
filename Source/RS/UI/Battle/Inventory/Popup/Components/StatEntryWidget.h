// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "RS/UI/Core/CharacterBoundUserWidget.h"
#include "StatEntryWidget.generated.h"

class UImage;
class UTextBlock;
/**
 *
 */
UCLASS()
class RS_API UStatEntryWidget : public UCharacterBoundUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> StatIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
