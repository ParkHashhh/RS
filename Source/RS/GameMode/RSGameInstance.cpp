// Fill out your copyright notice in the Description page of Project Settings.


#include "RSGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "RS/RSUtils.h"
#include "Components/AudioComponent.h"  
#include "Sound/SoundBase.h"

void URSGameInstance::Init()
{
	Super::Init();

	BGMComponent = NewObject<UAudioComponent>(this);
	BGMComponent->bAutoDestroy = false;
	BGMComponent->bIsUISound = true;       
	BGMComponent->bIgnoreForFlushing = true; 
	BGMComponent->SetSound(BGMSound);
	BGMComponent->Play();
}

void URSGameInstance::LevelLoading(const FName& LevelName, const FString& PackagePath, const FString& Options,
                                   bool bListen, bool IsServerTravel, bool bStoreData)
{
	NextLevelName = LevelName;
	NextLevelPackagePath = PackagePath;
	NextLevelOptions = Options;
	bNextLevelIsListen = bListen;
	bIsServerTravel = IsServerTravel;

	if (IsServerTravel)
	{
		RSUtils::ServerTravelWithDataStorage(GetWorld(), TEXT("Loading"), bStoreData, true, false);
	}
	else
	{
		RSUtils::OpenLevelWithDataStorage(GetWorld(), TEXT("Loading"), bStoreData, true, FString(TEXT("")));
	}
}

void URSGameInstance::StopBGM()
{
	if (IsValid(BGMComponent))
	{
		BGMComponent->Stop();
	}
}
