#include "RSUtils.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "RSTypes.h"

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

FText RSUtils::GetStatDescription(const FStatDisplayTableInfo& StatInfo, const TArray<FFormatArgumentValue>& Arguments)
{
	FFormatOrderedArguments Args;
	for (const auto& Arg : Arguments)
	{
		Args.Add(Arg);
	}

	return FText::Format(StatInfo.DescriptionFormat, Args);
}
