// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "StageObjectiveEntryWidget.generated.h"

class UStageObjectiveEntryObject;
class UImage;
class UTextBlock;
/**
 *
 */
UCLASS()
class RS_API UStageObjectiveEntryWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ProgressText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ProgressBar;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UStageObjectiveEntryObject> EntryObject;

public:
	void SetProgressText(const FText& InProgressText) const;

	void SetProgressRatio(float ProgressRatio) const;

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

private:
	void Init(const FText& Title, const FText& Description, const FText& InProgressText, float ProgressRatio) const;

	void OnObjectiveProgressChanged() const;
};
