#pragma once

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
}
