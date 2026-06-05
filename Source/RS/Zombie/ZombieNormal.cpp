// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal.h"
#include "GAS/ZombieStandardAttributeSet.h"

AZombieNormal::AZombieNormal()
{
	AttributeSet = CreateDefaultSubobject<UZombieStandardAttributeSet>(TEXT("Attribute"));
	UZombieStandardAttributeSet* StandardAttri = Cast<UZombieStandardAttributeSet>(AttributeSet);
	if (IsValid(StandardAttri))
	{
		ASC->AddAttributeSetSubobject<UZombieStandardAttributeSet>(StandardAttri);
	}
}

void AZombieNormal::BeginPlay()
{
	Super::BeginPlay();

	if (!MeshArray.IsEmpty())
	{
		int32 Index = FMath::RandRange(0, MeshArray.Num() - 1);

		if (MeshArray[Index])
		{
			Mesh->SetSkeletalMesh(MeshArray[Index]);
		}
	}

	NormalAttributeSet = Cast<UZombieStandardAttributeSet>(AttributeSet);
}
