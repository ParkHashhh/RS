// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "StageResultStatEntryWidget.generated.h"

class UTextBlock;
/**
 *
 */
UCLASS()
class RS_API UStageResultStatEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StatNameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StatValueText;

protected:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
};
