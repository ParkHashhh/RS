// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/CharacterBoundUserWidget.h"
#include "CrosshairWidget.generated.h"

struct FOnAttributeChangeData;
class APlayerCharacterBase;
class ARSPlayerController;
class UImage;

/**
 * WB_Crosshair의 RadiusModifier 기본값 설정하여 Crosshair의 크기를 조절할 수 있도록 함
 */
UCLASS()
class RS_API UCrosshairWidget : public UCharacterBoundUserWidget
{
	GENERATED_BODY()

public:
	UCrosshairWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly, Category="Crosshair|Center|Asset")
	TObjectPtr<UImage> Crosshair_C;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly, Category="Crosshair|Top|Asset")
	TObjectPtr<UImage> Crosshair_T;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Top|Translation")
	FVector2D Translation_T_Hip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Top|Translation")
	FVector2D Translation_T_Aim;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly, Category="Crosshair|Bottom|Asset")
	TObjectPtr<UImage> Crosshair_B;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Bottom|Translation")
	FVector2D Translation_B_Hip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Bottom|Translation")
	FVector2D Translation_B_Aim;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly, Category="Crosshair|Left|Asset")
	TObjectPtr<UImage> Crosshair_L;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Left|Translation")
	FVector2D Translation_L_Hip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Left|Translation")
	FVector2D Translation_L_Aim;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly, Category="Crosshair|Right|Asset")
	TObjectPtr<UImage> Crosshair_R;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Right|Translation")
	FVector2D Translation_R_Hip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair|Right|Translation")
	FVector2D Translation_R_Aim;

	// TArray<FAttributeDelegateBinding> AttributeDelegateBindings;

protected:
	virtual bool TryBindCharacterDelegates(APlayerCharacterBase* Character) override;
	virtual void UnbindCharacterDelegates() override;

	UFUNCTION(BlueprintImplementableEvent, DisplayName="On Aim Alpha Changed", Category="Crosshair")
	void K2_OnAimAlphaChanged(float Alpha);

private:
	void OnAimAlphaChanged(float Alpha);
};
