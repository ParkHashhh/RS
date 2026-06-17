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


		if (PlayerChar && PlayerChar->GetWeaponComponent())
		{
			LeftHandTransform = PlayerChar->GetWeaponComponent()->GetSocketTransform(TEXT("b_LeftHand"), RTS_World);
		}
		// 캐릭터가 바라보는 방향과 실제 이동 속도 방향의 차이를 계산합니다.
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

		bIsMoveKey = PlayerChar->GetIsMovekey();

		float Acceleration = Movement->GetCurrentAcceleration().Length();

		Accelerating = Acceleration > 0.f;

		RS_PRINT_SCREEN(0.f, FColor::Red, TEXT("Pitch : %.2f"), ViewPitch);
		RS_PRINT_SCREEN(0.f, FColor::Red, TEXT("Direction : %.2f"), Direction);
		RS_PRINT_SCREEN(0.f, FColor::Red, TEXT("Accelerating : %.2f"), Acceleration);

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
}


void UPlayerAnimInstance::MontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
}


void UPlayerAnimInstance::AnimNotify_AttackEnd()
{
}

void UPlayerAnimInstance::AnimNotify_AttackRay()
{
	TObjectPtr<APlayerCharacterBase> PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());

	if (IsValid(PlayerChar) && PlayerChar->IsAttacking())
	{
		PlayerChar->ShootAttackRay();
	}
}
