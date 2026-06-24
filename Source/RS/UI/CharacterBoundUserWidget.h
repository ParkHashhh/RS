// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RSUserWidget.h"
#include "RS/RSTypes.h"
#include "CharacterBoundUserWidget.generated.h"

class UDataSubsystem;
class APlayerCharacterBase;
/**
 *
 */
UCLASS()
class RS_API UCharacterBoundUserWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UCharacterBoundUserWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerCharacterBase> CachedCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UDataSubsystem> DataSubsystem;

protected:
	TArray<FAttributeDelegateBinding> AttributeDelegateBindings;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 자식에서
	// GetCachedCharacter()->델리게이트변수.AddUObject(this, &ThisClass::콜백);
	//
	// if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	// {
	// 	FGameplayAttribute Attribute = UPlayerAttributeSet::Get어트리뷰트();
	// 	FDelegateHandle Handle = ASC->GetGameplayAttributeValueChangeDelegate(Attribute)
	// 	                            .AddUObject(this, &ThisClass::콜백);
	// 	AttributeDelegateBindings.Emplace(Attribute, Handle);
	//
	// 	FOnAttributeChangeData InitialData;
	// 	InitialData.Attribute = Attribute;
	// 	InitialData.NewValue = ASC->GetNumericAttribute(Attribute);
	// 	InitialData.OldValue = InitialData.NewValue;
	// 	콜백(InitialData);
	// }
	virtual bool TryBindCharacterDelegates(APlayerCharacterBase* Character);
	virtual void UnbindCharacterDelegates();

	FORCEINLINE APlayerCharacterBase* GetCachedCharacter() const
	{
		return CachedCharacter.Get();
	}

	FORCEINLINE UDataSubsystem* GetDataSubsystem() const
	{
		return DataSubsystem;
	}

private:
	bool TryBindCharacter(APlayerCharacterBase* Character);
	void OnGameplayReady(bool bIsReady);
};
