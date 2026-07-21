// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../RSUserWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "MapSelectCardWidget.generated.h"

/**
 * 
 */
class UMapSelectCardWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapCardClicked, UMapSelectCardWidget*, ClickedCard);

UCLASS()
class RS_API UMapSelectCardWidget : public URSUserWidget
{
	GENERATED_BODY()
public:
	UMapSelectCardWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// 디자이너에서 만든 버튼/보더 이름과 정확히 일치해야 함
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>  HitButton;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SelectBorder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MapCard")
	FName LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MapCard")
	FString LevelPackagePath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MapCard")
	FLinearColor SelectedTint = FLinearColor(1.f, 0.85f, 0.4f, 1.f);

	bool bIsSelected = false;

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleHitAreaClicked();
public:
	FORCEINLINE FName GetLevelName() const
	{
		return LevelName;
	}
	FORCEINLINE const FString& GetLevelPackagePath() const
	{
		return LevelPackagePath;
	}
	FORCEINLINE bool IsSelected() const
	{
		return bIsSelected;
	}
public:
	UPROPERTY(BlueprintAssignable, Category = "MapCard")
	FOnMapCardClicked OnCardClicked;

	UFUNCTION(BlueprintCallable, Category = "MapCard")
	void SetSelected(bool bSelected);
};
