#include "RSUtils.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "RSTags.h"
#include "RSTypes.h"
#include "Data/Subsystem/DataStorageSubsystem.h"
#include "Kismet/GameplayStatics.h"

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

const FGameplayAttribute* RSUtils::GetAttributeFromTag(const FGameplayTag& AttributeTag,
                                                       const TSubclassOf<UGameplayEffect>& MasterEffectClass)
{
	if (!AttributeTag.IsValid() || !MasterEffectClass)
	{
		return nullptr;
	}

	const auto* DefaultGE = MasterEffectClass->GetDefaultObject<UGameplayEffect>();
	if (!DefaultGE)
	{
		return nullptr;
	}

	for (const FGameplayModifierInfo& Modifier : DefaultGE->Modifiers)
	{
		if (Modifier.ModifierMagnitude.GetMagnitudeCalculationType() ==
			EGameplayEffectMagnitudeCalculation::SetByCaller)
		{
			FSetByCallerFloat SetByCallerDetails = Modifier.ModifierMagnitude.GetSetByCallerFloat();
			if (SetByCallerDetails.DataTag == AttributeTag)
			{
				return &Modifier.Attribute;
			}
		}
	}

	return nullptr;
}

int32 RSUtils::GetStatPriority(const FGameplayTag& StatTag)
{
	if (StatTag == RSTags::Common::MaxHP) return 0;
	if (StatTag == RSTags::Common::AttackPower) return 1;
	if (StatTag == RSTags::Common::AttackSpeed) return 2;
	if (StatTag == RSTags::Common::DefensePower) return 3;
	if (StatTag == RSTags::Common::CriticalChance) return 4;
	if (StatTag == RSTags::Common::CriticalDamage) return 5;
	if (StatTag == RSTags::Common::MaxRange) return 6;
	if (StatTag == RSTags::Common::MaxAmmo) return 7;
	if (StatTag == RSTags::Common::ShotSpread) return 8;
	if (StatTag == RSTags::Common::WalkSpeed) return 9;
	if (StatTag == RSTags::Common::DashSpeed) return 10;

	return INT_MAX;
}

void RSUtils::OpenLevelWithDataStorage(const UObject* WorldContextObject, FName LevelName, bool bStoreData,
	bool bAbsolute, FString Options)
{
	if (bStoreData)
	{
		if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (const UGameInstance* GI = World->GetGameInstance())
			{
				if (auto* StoreSystem = GI->GetSubsystem<UDataStorageSubsystem>())
				{
					StoreSystem->StoreAllPlayerInventory();
					StoreSystem->StoreAllPlayerStageResultStat();
				}
			}
		}
	}

	UGameplayStatics::OpenLevel(WorldContextObject, LevelName, bAbsolute, Options);
}

bool RSUtils::ServerTravelWithDataStorage(UWorld* World, const FString& InURL, bool bStoreData, bool bAbsolute,
                                        bool bShouldSkipGameNotify)
{
	if (!World)
	{
		return false;
	}

	if (bStoreData)
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (auto* StoreSystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				StoreSystem->StoreAllPlayerInventory();
				StoreSystem->StoreAllPlayerStageResultStat();
			}
		}
	}

	return World->ServerTravel(InURL, bAbsolute, bShouldSkipGameNotify);
}
