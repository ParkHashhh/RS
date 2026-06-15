// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/RSUserWidget.h"
#include "CrosshairWidget.generated.h"

class ARSPlayerController;
class UImage;
/**
 *
 */
UCLASS()
class RS_API UCrosshairWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UCrosshairWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Crosshair_C;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Crosshair_T;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Crosshair_B;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Crosshair_L;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Crosshair_R;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float BaseSpread;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float AimSpreadModifer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float MoveSpreadModifer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float FireSpreadModifer;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float CurrentSpread;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float TargetSpread;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float MoveTime;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true), Category="Crosshair")
	float ElapsedTime;

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_GetCachedPlayerController, meta=(AllowPrivateAccess=true))
	TObjectPtr<ARSPlayerController> CachedPlayerController;

	FDelegateHandle WaitPlayerControllerHandle;

public:
	ARSPlayerController* GetCachedPlayerController();

	UFUNCTION(BlueprintGetter, DisplayName="Get Cached Player Controller")
	ARSPlayerController* K2_GetCachedPlayerController()
	{
		return GetCachedPlayerController();
	}

	bool TryCachePlayerController();


protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	void OnPlayerControllerReady(ARSPlayerController* PlayerController);
};
