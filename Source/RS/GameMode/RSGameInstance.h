// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RSGameInstance.generated.h"

/**
 *
 */
UCLASS()
class RS_API URSGameInstance : public UGameInstance
{
	GENERATED_BODY()

private:
	FName NextLevelName;

public:
	UPROPERTY()
	FName SelectedTextureName;

	UPROPERTY()
	FName SelectedSkillName;

	UPROPERTY()
	FString NextLevelPackagePath;

	UPROPERTY()
	bool bNextLevelIsListen;

	UPROPERTY()
	bool bIsServerTravel;

	UPROPERTY()
	FString NextLevelOptions;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase> BGMSound;

	UPROPERTY()
	TObjectPtr<UAudioComponent> BGMComponent;

public:
	virtual void Init() override;

	void LevelLoading(const FName& LevelName, const FString& PackagePath, const FString& Options, bool bListen = false,
	                  bool IsServerTravel = false, bool
	                  bStoreInventory = false);

public:
	void StopBGM();
	bool IsListen() const
	{
		return bNextLevelIsListen;
	}

	bool IsServerTravel() const
	{
		return bIsServerTravel;
	}

	FName GetNextLevelName() const
	{
		return NextLevelName;
	}

	const FString& GetNextLevelOptions() const
	{
		return NextLevelOptions;
	}

	const FString& GetNextLevelPackagePath() const
	{
		return NextLevelPackagePath;
	}
};
