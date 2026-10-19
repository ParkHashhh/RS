// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RSGameMode.generated.h"

/**
 *
 */
UCLASS()
class RS_API ARSGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameInstance> CachedGameInstance;

	TMap<TSubclassOf<UGameInstanceSubsystem>, TWeakObjectPtr<UGameInstanceSubsystem>> SubsystemCache;

protected:
	UGameInstance* GetCachedGameInstance()
	{
		if (!CachedGameInstance.IsValid())
		{
			CachedGameInstance = GetGameInstance();
		}

		return CachedGameInstance.Get();
	}

	template <typename T>
	T* GetCachedSubsystem()
	{
		static_assert(TIsDerivedFrom<T, UGameInstanceSubsystem>::IsDerived, "T must be derived from UGameInstanceSubsystem");

		const auto Key = TSubclassOf<UGameInstanceSubsystem>(T::StaticClass());

		if (!SubsystemCache.Contains(Key) || !SubsystemCache[Key].IsValid())
		{
			if (const UGameInstance* GI = GetCachedGameInstance())
			{
				if (auto Subsystem = GI->GetSubsystem<T>())
				{
					SubsystemCache.Emplace(Key, Subsystem);
				}
			}
		}

		return Cast<T>(SubsystemCache.FindRef(Key));
	}
};
