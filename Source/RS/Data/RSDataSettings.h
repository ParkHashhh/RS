// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RSDataSettings.generated.h"

class UPlayerDefaultInfoTableAsset;
class UItemInfoTableAsset;
class UMiscInfoTableAsset;
class UStatDisplayInfoTableAsset;
class UZombieDefaultInfoTableAsset;
class UStageObjectiveInfoTableAsset;
class UStageInfoTableAsset;
class UGameplayEffect;

/**
 * 프로젝트 전체에서 사용되는 데이터 관련 설정입니다.
 * 프로젝트 세팅 -> Game -> RS Data Settings에서 편집할 수 있습니다.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="RS Data Settings"))
class RS_API URSDataSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	URSDataSettings();

	// 플레이어 캐릭터의 기본 정보가 담긴 테이블 애셋
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UPlayerDefaultInfoTableAsset> PlayerDefaultInfoTableAsset;

	// 아이템 정보가 담긴 데이터 테이블 애셋
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UItemInfoTableAsset> ItemInfoTableAsset;

	// 기타 데이터 테이블 애셋
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UMiscInfoTableAsset> MiscInfoTableAsset;

	// 스탯 설명 테이블 애셋
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UStatDisplayInfoTableAsset> StatDisplayInfoTableAsset;

	// 좀비의 기본 정보가 담긴 테이블 애셋
	UPROPERTY(Config, EditAnywhere, Category = "Data")
	TSoftObjectPtr<UZombieDefaultInfoTableAsset> ZombieDefaultInfoTableAsset;

	UPROPERTY(Config, EditAnywhere, Category = "Data")
	TSoftObjectPtr<UStageObjectiveInfoTableAsset> StageObjectiveInfoTableAsset;

	UPROPERTY(Config, EditAnywhere, Category = "Data")
	TSoftObjectPtr<UStageInfoTableAsset> StageInfoTableAsset;

	// 모든 장착 아이템이 공통으로 사용하는 마스터 GE 클래스.
	// 예전엔 itemtable.csv의 행마다 저장했는데, 전 행이 항상 동일한 값을 써서
	// DataTable 언버전드 프로퍼티 스키마 계산에서 간헐적으로 깨지는 문제가 있어 여기로 옮김.
	UPROPERTY(Config, EditAnywhere, Category = "Data")
	TSoftClassPtr<UGameplayEffect> MasterItemEffectClass;
};
