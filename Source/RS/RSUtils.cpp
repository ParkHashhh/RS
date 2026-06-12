#include "RSUtils.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"

UAbilitySystemComponent* RSUtils::FindASC(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	if (const auto* Interface = Cast<IAbilitySystemInterface>(Actor))
	{
		return Interface->GetAbilitySystemComponent();
	}

	return Actor->FindComponentByClass<UAbilitySystemComponent>();
}
