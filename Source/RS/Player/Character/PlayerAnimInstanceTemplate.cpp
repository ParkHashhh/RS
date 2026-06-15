// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstanceTemplate.h"
#include "RS/LogCategory.h"


void UPlayerAnimInstanceTemplate::NativeBeginPlay()
{
	Super::NativeBeginPlay();

}

void UPlayerAnimInstanceTemplate::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	RS_PRINT_SCREEN(-0.1f, FColor::Red, TEXT("MoveSpeed : %.2f"), MoveSpeed);

}
