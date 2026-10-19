// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/Character/SelectPawn.h"

// Sets default values
ASelectPawn::ASelectPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SelectMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));

	SetRootComponent(SelectMesh);
	SelectMesh->SetCollisionProfileName(TEXT("Player"));


}

// Called when the game starts or when spawned
void ASelectPawn::BeginPlay()
{
	Super::BeginPlay();
	if (SubSkillNames.IsValidIndex(CurrentNameIndex))
	{
		CurrentSkillName = SubSkillNames[CurrentNameIndex];
	}
}

void ASelectPawn::NextTexture()
{
	if (!SelectMeshes.Num())
	{
		return;
	}
	CurrentMeshIndex++;
	if (CurrentMeshIndex >= SelectMeshes.Num())
	{
		CurrentMeshIndex = 0;
	}
	SelectMesh->SetSkeletalMesh(SelectMeshes[CurrentMeshIndex]);
}

void ASelectPawn::PrevTexture()
{
	if (!SelectMeshes.Num())
	{
		return;
	}
	CurrentMeshIndex--;
	if (CurrentMeshIndex < 0)
	{
		CurrentMeshIndex = SelectMeshes.Num() - 1;
	}
	SelectMesh->SetSkeletalMesh(SelectMeshes[CurrentMeshIndex]);
}

void ASelectPawn::NextSkill()
{
	if (!SubSkillNames.Num())
	{
		return;
	}
	CurrentNameIndex++;
	if (CurrentNameIndex >= SubSkillNames.Num())
	{
		CurrentNameIndex = 0;
	}
	CurrentSkillName = SubSkillNames[CurrentNameIndex];
}

void ASelectPawn::PrevSkill()
{
	if (!SubSkillNames.Num())
	{
		return;
	}
	CurrentNameIndex--;

	if (CurrentNameIndex < 0)
	{
		CurrentNameIndex = SubSkillNames.Num() - 1;
	}
	CurrentSkillName = SubSkillNames[CurrentNameIndex];

}

FName ASelectPawn::GetCurrentTextureName()
{
	if (SelectMeshes.IsValidIndex(CurrentMeshIndex) && SelectMeshes[CurrentMeshIndex])
	{
		return SelectMeshes[CurrentMeshIndex]->GetFName();
	}
	return NAME_None;
}

// Called every frame
void ASelectPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASelectPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

