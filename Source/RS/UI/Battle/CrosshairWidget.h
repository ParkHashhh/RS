// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSTypes.h"
#include "RS/UI/RSUserWidget.h"
#include "CrosshairWidget.generated.h"

struct FOnAttributeChangeData;
class APlayerCharacterBase;
class ARSPlayerController;
class UImage;

/**
 * WB_Crosshair의 RadiusModifier 기본값 설정하여 Crosshair의 크기를 조절할 수 있도록 함
 */
UCLASS()
class RS_API UCrosshairWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UCrosshairWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly)
	TObjectPtr<UImage> Crosshair_C;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly)
	TObjectPtr<UImage> Crosshair_T;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly)
	TObjectPtr<UImage> Crosshair_B;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly)
	TObjectPtr<UImage> Crosshair_L;

	UPROPERTY(meta=(BindWidget, AllowPrivateAccess=true), BlueprintReadOnly)
	TObjectPtr<UImage> Crosshair_R;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerCharacterBase> CachedCharacter;

	TArray<FAttributeDelegateBinding> AttributeDelegateBindings;

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, DisplayName="On Shot Spread Radius Changed", Category="Crosshair")
	void K2_OnShotSpreadRadiusChanged(float Spread);

private:
	void OnGameplayReady(bool bIsReady);

	void BindCharacterDelegates(APlayerCharacterBase* Character);
	void UnbindCharacterDelegates();

	void OnShotSpreadRadiusChanged(const FOnAttributeChangeData& Data);
};
