// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RSGameMode.h"
#include "StartGameMode.generated.h"

class UStageResultStatEntryObject;
class UListView;
/**
 *
 */
UCLASS()
class RS_API AStartGameMode : public ARSGameMode
{
	GENERATED_BODY()

public:
	AStartGameMode();

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UListView> StageResultStatListView;

	UPROPERTY()
	TArray<TObjectPtr<UStageResultStatEntryObject>> StageResultStatEntryObjects;

public:
	void Init();
};
