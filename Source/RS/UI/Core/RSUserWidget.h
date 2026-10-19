// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "RSUserWidget.generated.h"

class UUISubsystem;
class UDataSubsystem;
/**
 *
 */
UCLASS()
class RS_API URSUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	URSUserWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, BlueprintGetter=K2_GetWidgetName, meta=(AllowPrivateAccess = true))
	FName WidgetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	bool bIsCombatWidget = false;

	TMap<FName, TWeakObjectPtr<UWidgetAnimation>> Animations;

public:
	FORCEINLINE FName GetWidgetName() const
	{
		return WidgetName;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Widget Name")
	FName K2_GetWidgetName() const
	{
		return GetWidgetName();
	}

	void PlayWidgetAnimation(FName AnimationName, float StartAtTime = 0.0f, float PlaybackSpeed = 1.f,
	                         bool bForward = true, bool bRestoreState = false, int32 NumberOfLoops = 1);

	UFUNCTION(BlueprintPure)
	bool IsCombatWidget() const
	{
		return bIsCombatWidget;
	}

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	template <typename T>
	const T* GetSubsystem() const
	{
		if constexpr (std::is_base_of_v<ULocalPlayerSubsystem, T>)
		{
			if (const ULocalPlayer* LP = GetOwningLocalPlayer())
			{
				return LP->GetSubsystem<T>();
			}

			return nullptr;
		}
		else if constexpr (std::is_base_of_v<UGameInstanceSubsystem, T>)
		{
			if (const UGameInstance* GI = GetGameInstance())
			{
				return GI->GetSubsystem<T>();
			}

			return nullptr;
		}
		else
		{
			static_assert(sizeof(T) == 0, "T must be derived from ULocalPlayerSubsystem or UGameInstanceSubsystem");
			return nullptr;
		}
	}
};
