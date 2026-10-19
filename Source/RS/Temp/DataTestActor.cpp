// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTestActor.h"

#include "RS/Core/LogCategory.h"
#include "RS/Core/RSTypes.h"
#include "RS/Data/Subsystem/DataSubsystem.h"


// Sets default values
ADataTestActor::ADataTestActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADataTestActor::BeginPlay()
{
	Super::BeginPlay();

	const UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);

	auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);
	DataSubsystem->ExecuteOrBindOnItemInfoLoaded(this, &ThisClass::OnItemInfoLoaded);
}

// Called every frame
void ADataTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADataTestActor::OnItemInfoLoaded()
{
	const UGameInstance* GameInstance = GetGameInstance();
	check(GameInstance);

	const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	if (const auto* PotionItem = DataSubsystem->GetItemTableInfo(FName(TEXT("hp_potion"))))
	{
		RS_PRINT_SCREEN(3.f, FColor::Cyan, TEXT("%s"), *PotionItem->Name.ToString());
	}
	else
	{
		RS_PRINT_SCREEN(3.f, FColor::Red, TEXT("Item not found: hp_potion"));
	}
}

