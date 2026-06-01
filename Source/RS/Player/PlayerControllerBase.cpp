// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"

APlayerControllerBase::APlayerControllerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 마우스 커서를 보이게 한다.
	bShowMouseCursor = false;


}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	// 입력모드를 세팅한다
	// FInputModeGameOnly
	// FInputModeGameAndUI
	// FInputModeUIOnly

	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);

}
void APlayerControllerBase::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

}
void APlayerControllerBase::OnUnPossess()
{
	Super::OnUnPossess();

}
void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
