// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal.h"

AZombieNormal::AZombieNormal()
{
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
}
