// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpecial.h"
#include "GAS/ZombieStandardAttributeSet.h"

AZombieSpecial::AZombieSpecial()
{
	AttributeSet = CreateDefaultSubobject<UZombieStandardAttributeSet>(TEXT("Attribute"));
	UZombieStandardAttributeSet* StandardAttri = Cast<UZombieStandardAttributeSet>(AttributeSet);
	if (IsValid(StandardAttri))
	{
		ASC->AddAttributeSetSubobject<UZombieStandardAttributeSet>(StandardAttri);
	}
}

void AZombieSpecial::BeginPlay()
{
	Super::BeginPlay();

	SpecialAttributeSet = Cast<UZombieStandardAttributeSet>(AttributeSet);
}
