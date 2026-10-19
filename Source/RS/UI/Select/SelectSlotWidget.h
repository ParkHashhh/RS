// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "Components/TextBlock.h"
#include "OnlineSessionSettings.h"
#include "SelectSlotWidget.generated.h"

/**
 *
 */
UCLASS()
class RS_API USelectSlotWidget : public URSUserWidget
{
	GENERATED_BODY()
public:
	USelectSlotWidget(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TextureLeftButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> TextureRightButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillLeftButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> SkillRightButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextureText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SkillText;

	TObjectPtr<class ASelectPawn> CurrentSelectPawn;

	FDelegateHandle FindSessionsCompleteHandle;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	void OnFindSessionsCompleted(bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);
	void ClearFindSessionsDelegate();


public:
	UFUNCTION()
	void TextureLeftButtonClick();

	UFUNCTION()
	void TextureRightClick();

	UFUNCTION()
	void SkillLeftButtonClick();

	UFUNCTION()
	void SkillRightButtonClick();

	UFUNCTION()
	void PlayButtonClick();

private:
	void StartButtonClick();
	void MultiStartButtonClick();

public:

	UFUNCTION()
	void TestJoinLocalHost();
public:
	void PickSelectPawn(class ASelectPawn* SelectPawn);
};
