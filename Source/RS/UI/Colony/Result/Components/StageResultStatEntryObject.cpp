// Fill out your copyright notice in the Description page of Project Settings.


#include "StageResultStatEntryObject.h"

void UStageResultStatEntryObject::Initialize(EStageResultStatType InStatType, float InStatValue)
{
	StatType = InStatType;
	StatValue = InStatValue;
}

FText UStageResultStatEntryObject::GetStatNameText() const
{
	switch (StatType)
	{
	case EStageResultStatType::Kill:
		return NSLOCTEXT("StageResult", "StatKill", "Kills");
	case EStageResultStatType::Time:
		return NSLOCTEXT("StageResult", "StatTime", "Time");
	default:
		ensureMsgf(false, TEXT("Unhandled EStageResultStatType: %d"), static_cast<int32>(StatType));
		return FText::GetEmpty();
	}
}

FText UStageResultStatEntryObject::GetStatValueText() const
{
	switch (StatType)
	{
	case EStageResultStatType::Kill:
		{
			const int32 KillCount = FMath::RoundToInt32(StatValue);
			return FText::AsNumber(KillCount);
		}

	case EStageResultStatType::Time:
		{
			const int32 TotalSeconds = FMath::Max(0, FMath::FloorToInt(StatValue));

			const auto [Quot, Rem] = div(TotalSeconds, 60);
			const int32 Minutes = Quot;
			const int32 Seconds = Rem;

			FNumberFormattingOptions NumberFormat;
			NumberFormat.MinimumIntegralDigits = 2;
			NumberFormat.MaximumIntegralDigits = 2;

			const FFormatNamedArguments Args =
			{
				{TEXT("Minutes"), FText::AsNumber(Minutes, &NumberFormat)},
				{TEXT("Seconds"), FText::AsNumber(Seconds, &NumberFormat)}
			};

			return FText::Format(NSLOCTEXT("StageResult", "TimeFormat", "{Minutes}:{Seconds}"), Args);
		}

	default:
		ensureMsgf(false, TEXT("Unhandled EStageResultStatType: %d"), static_cast<int32>(StatType));
		return FText::GetEmpty();
	}
}
