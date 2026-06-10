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

		// 캐릭터가 바라보는 방향과 실제 이동 속도 방향의 차이를 계산합니다.
		Direction = CalculateDirection(Movement->Velocity, PlayerChar->GetActorRotation());

		// 공중 상태 (IsFalling)
		IsInAir = Movement->IsFalling();

		//  앉기 상태
		IsCrouching = Movement->IsCrouching();
	}

}
