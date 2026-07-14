#pragma once
#include "GameplayTagContainer.h"

class UGameplayEffect;
struct FGameplayTag;
struct FGameplayAttribute;
struct FStatDisplayTableInfo;
class UAbilitySystemComponent;

namespace RSUtils
{
	UAbilitySystemComponent* FindASC(const AActor* Actor);

	template <typename TObject>
	void ExecuteOrBind(bool bReady, FSimpleMulticastDelegate& Delegate, TObject* Object, void (TObject::*Callback)())
	{
		static_assert(TIsDerivedFrom<TObject, UObject>::IsDerived, "T must be derived from UObject");

		if (!Object || !Callback)
		{
			return;
		}

		if (bReady)
		{
			(Object->*Callback)();
		}
		else
		{
			Delegate.AddUObject(Object, Callback);
		}
	}

	template <typename T>
	const T* GetTableInfo(const FDataTableRowHandle& RowHandle, const TCHAR* Context = TEXT(""))
	{
		if (!RowHandle.DataTable)
		{
			return nullptr;
		}

		return RowHandle.DataTable->FindRow<T>(RowHandle.RowName, Context);
	}

	FText GetStatDescription(const FStatDisplayTableInfo& StatInfo, const TArray<FFormatArgumentValue>& Arguments);

	FORCEINLINE bool IsEven(int32 Value)
	{
		return (Value & 1) == 0;
	}

	FORCEINLINE bool IsOdd(int32 Value)
	{
		return (Value & 1) != 0;
	}

	const FGameplayAttribute* GetAttributeFromTag(const FGameplayTag& AttributeTag,
	                                              const TSubclassOf<UGameplayEffect>& MasterEffectClass);

	int32 GetStatPriority(const FGameplayTag& StatTag);

	void OpenLevelWithInventory(const UObject* WorldContextObject, FName LevelName, bool bStoreInventory = true, bool bAbsolute = true, FString Options = FString(TEXT("")));
}
