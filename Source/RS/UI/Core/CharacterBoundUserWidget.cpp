// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/UI/Core/CharacterBoundUserWidget.h"

#include "RS/Core/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "RS/Player/Subsystem/GameplaySubsystem.h"
#include "RS/Player/Character/PlayerCharacterBase.h"

UCharacterBoundUserWidget::UCharacterBoundUserWidget(const FObjectInitializer& ObjectInitializer) :
	URSUserWidget(ObjectInitializer)
{
}

void UCharacterBoundUserWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (const UGameInstance* GI = GetGameInstance())
	{
		DataSubsystem = GI->GetSubsystem<UDataSubsystem>();
	}

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			GameplaySubsystem->OnGameplayReady.AddUObject(this, &ThisClass::OnGameplayReady);
		}
	}
}

void UCharacterBoundUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			if (GameplaySubsystem->IsGameplayReady())
			{
				OnGameplayReady(true);
			}
		}
	}
}

void UCharacterBoundUserWidget::NativeDestruct()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			GameplaySubsystem->OnGameplayReady.RemoveAll(this);
		}
	}

	UnbindCharacterDelegates();

	Super::NativeDestruct();
}

bool UCharacterBoundUserWidget::TryBindCharacter(APlayerCharacterBase* Character)
{
	if (!Character || CachedCharacter == Character)
	{
		RS_LOG_DEBUG(LogRSUI, Error, TEXT("%s: Failed to bind character"), *LOG_LINE_INFO);
		return false;
	}

	// 기존 바인딩 해제
	UnbindCharacterDelegates();

	CachedCharacter = Character;

	return true;
}

void UCharacterBoundUserWidget::OnGameplayReady(bool bIsReady)
{
	if (bIsReady)
	{
		if (const ULocalPlayer* LP = GetOwningLocalPlayer())
		{
			if (const auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
			{
				TryBindCharacterDelegates(GameplaySubsystem->GetCachedCharacter());
			}
		}
	}
	else
	{
		UnbindCharacterDelegates();
	}
}

void UCharacterBoundUserWidget::UnbindCharacterDelegates()
{
	if (CachedCharacter.IsValid())
	{
		if (UAbilitySystemComponent* ASC = CachedCharacter->GetAbilitySystemComponent())
		{
			for (auto& [Attribute, Handle] : AttributeDelegateBindings)
			{
				if (Handle.IsValid())
				{
					ASC->GetGameplayAttributeValueChangeDelegate(Attribute).Remove(Handle);
				}
			}
		}
	}

	AttributeDelegateBindings.Empty();
	CachedCharacter.Reset();
}

bool UCharacterBoundUserWidget::TryBindCharacterDelegates(APlayerCharacterBase* Character)
{
	if (!TryBindCharacter(Character))
	{
		RS_LOG_DEBUG(LogRSUI, Error, TEXT("%s: Failed to bind character delegates"), *LOG_LINE_INFO);
		return false;
	}

	return true;
}
