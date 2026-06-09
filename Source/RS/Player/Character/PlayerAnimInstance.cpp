// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

}
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	TObjectPtr<APlayerCharacterBase>	PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());

	if (IsValid(PlayerChar))
	{
		UCharacterMovementComponent* Movement = PlayerChar->GetCharacterMovement();

		MoveSpeed = Movement->Velocity.Length();
	}

}
