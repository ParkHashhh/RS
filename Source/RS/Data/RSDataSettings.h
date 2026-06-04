// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RSDataSettings.generated.h"

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

	// 아이템 정보가 담긴 데이터 테이블 애셋 ID
	UPROPERTY(Config, EditAnywhere, Category="Data")
	FPrimaryAssetId ItemInfoTableAssetId;
};
