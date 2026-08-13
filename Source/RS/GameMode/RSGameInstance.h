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

	UPROPERTY()
	bool bIsSinglePlayerMode = true;
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

	//로딩 레벨에서 한번만 봄버 좀비가 터지게 하기 위한 불리언 변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bIsBomberNiagaraLoaded = false;


public:
public:
	void SetSinglePlayerMode(bool bInSinglePlayer) { bIsSinglePlayerMode = bInSinglePlayer; }
	bool IsSinglePlayerMode() const
	{
		return bIsSinglePlayerMode;
	}
	virtual void Init() override;

	void LevelLoading(const FName& LevelName, const FString& PackagePath, const FString& Options, bool bListen = false,
	                  bool IsServerTravel = false, bool
	                  bStoreData = false);

public:

	void StopBGM();
	void PlayBGM();
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
