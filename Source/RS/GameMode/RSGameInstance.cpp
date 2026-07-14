// Fill out your copyright notice in the Description page of Project Settings.


#include "RSGameInstance.h"
#include "Kismet/GameplayStatics.h"


void URSGameInstance::LevelLoading(const FName& LevelName, const FString& PackagePath, const FString& Options, bool bListen, bool IsServerTravel)
{
	NextLevelName = LevelName;
	NextLevelPackagePath = PackagePath;
	NextLevelOptions = Options;
	bNextLevelIsListen = bListen;
	bIsServerTravel = IsServerTravel;

	if (IsServerTravel)
	{
		GetWorld()->ServerTravel(TEXT("Loading"), true);
	}
	else
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("Loading"));
	}

}
