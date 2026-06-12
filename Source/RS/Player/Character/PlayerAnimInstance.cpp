// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"



void UPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	//OnMontageEnded.AddDynamic(this, &UPlayerAnimInstance::MontageEnd);
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	TObjectPtr<APlayerCharacterBase> PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());

	if (IsValid(PlayerChar))
	{
		UCharacterMovementComponent* Movement = PlayerChar->GetCharacterMovement();

		MoveSpeed = Movement->Velocity.Length();

		// 캐릭터가 바라보는 방향과 실제 이동 속도 방향의 차이를 계산합니다.
		//Direction = CalculateDirection(Movement->Velocity, PlayerChar->GetActorRotation());

		// UAnimInstance::CalculateDirection는 Deprecated 되어 대신 UKismetAnimationLibrary 사용
		Direction = UKismetAnimationLibrary::CalculateDirection(Movement->Velocity, PlayerChar->GetActorRotation());

		if (!PlayerChar->IsAttacking() || !PlayerChar->IsAiming() || !PlayerChar->IsDashing())
		{
			// 공중 상태 (IsFalling)
			bIsInAir = Movement->IsFalling();
		}
		//  앉기 상태
		bIsCrouching = Movement->IsCrouching();

		bIsAiming = PlayerChar->IsAiming();

		bIsAttacking = PlayerChar->IsAttacking();

		bIsDashing = PlayerChar->IsDashing();
	}
}

void UPlayerAnimInstance::PlayBaseAttackMontage()
{
	if (!BaseAttackMontage)
	{
		return;
	}
	if (Montage_IsPlaying(BaseAttackMontage))
	{
		return;
	}
	Montage_Play(BaseAttackMontage, 1.f);
	RS_PRINT_SCREEN(1, FColor::Yellow, TEXT("Attack Start"));
}


void UPlayerAnimInstance::MontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	//RS_PRINT_SCREEN(1, FColor::Yellow, TEXT("PlayerAnimInst MontageEnd"));
}


void UPlayerAnimInstance::AnimNotify_AttackEnd()
{
	RS_PRINT_SCREEN(5, FColor::Red, TEXT("PlayerAnimInst MontageEnd"));
}
