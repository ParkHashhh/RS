// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "InteractionWidget.generated.h"

class UImage;
class UPaperSprite;
/**
 *
 */
UCLASS()
class RS_API UInteractionWidget : public URSUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> InputKeyIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ProgressImage;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ProgressMaterial;

	UPROPERTY(EditDefaultsOnly, Category="Interaction|Input", meta=(AllowPrivateAccess=true))
	TObjectPtr<UPaperSprite> InputSpriteEnabled;

	UPROPERTY(EditDefaultsOnly, Category="Interaction|Input", meta=(AllowPrivateAccess=true))
	TObjectPtr<UPaperSprite> InputSpriteDisabled;

	bool bIsProgressEnable = false;

public:
	UInteractionWidget(const FObjectInitializer& ObjectInitializer);

	void Init(bool bInteractable = true);

	void SetInputEnabled(bool bEnabled) const;
	void SetProgress(float Progress) const;

	void SetProgressEnable(bool bEnable);

protected:
	virtual void NativeOnInitialized() override;
};
