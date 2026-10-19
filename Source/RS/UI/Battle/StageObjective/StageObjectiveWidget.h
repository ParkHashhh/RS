// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "StageObjectiveWidget.generated.h"

class UStageObjectiveEntryWidget;
class UStageObjectiveEntryObject;
class UListView;
/**
 *
 */
UCLASS()
class RS_API UStageObjectiveWidget : public URSUserWidget
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStageObjectiveEntryWidget> StageObjectiveEntryWidgetClass;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> StageObjectiveEntryListView;

	UPROPERTY()
	TArray<TObjectPtr<UStageObjectiveEntryObject>> StageObjectiveEntryObjects;

public:
	void Init();

protected:
	virtual void NativeConstruct() override;

private:
	void RefreshContents();
};
