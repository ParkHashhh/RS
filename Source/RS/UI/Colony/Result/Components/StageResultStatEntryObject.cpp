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
			// 초 단위 float를 FTimespan으로 변환하여 mm:ss 포맷팅
			const FTimespan TimeSpan = FTimespan::FromSeconds(StatValue);

			const FFormatNamedArguments Args = {
				{TEXT("Minutes"), FText::AsNumber(TimeSpan.GetMinutes())},
				{TEXT("Seconds"), FText::AsNumber(TimeSpan.GetSeconds())}
			};

			// {Minutes:02} 패딩 옵션으로 2자리 수(01, 02...) 유지
			return FText::Format(NSLOCTEXT("StageResult", "TimeFormat", "{Minutes:02}:{Seconds:02}"), Args);
		}

	default:
		ensureMsgf(false, TEXT("Unhandled EStageResultStatType: %d"), static_cast<int32>(StatType));
		return FText::GetEmpty();
	}
}
