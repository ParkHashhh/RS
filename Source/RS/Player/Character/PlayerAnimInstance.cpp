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

// TODO: 추후 PlayerCharacter쪽에서 값이 변할 때 AnimInstance 값을 변경하는 쪽으로 로직 수정. Update 로직을 굳이?
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());
	if (PlayerChar)
	{
		const UCharacterMovementComponent* Movement = PlayerChar->GetCharacterMovement();

		MoveSpeed = Movement->Velocity.Length();


		if (PlayerChar && PlayerChar->GetWeaponComponent())
		{
			LeftHandTransform = PlayerChar->GetWeaponComponent()->GetSocketTransform(TEXT("b_LeftHand"), RTS_World);
			LeftElbowTransform = PlayerChar->GetMesh()->GetSocketTransform(TEXT("lowerarm_l"), RTS_World);
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

		bIsAiming = PlayerChar->IsAiming();

		bIsAttacking = PlayerChar->IsAttacking();

		bIsDashing = PlayerChar->IsDashing();

		bIsMoveKey = PlayerChar->IsMoveKeyPressed();

		bIsReloading = PlayerChar->IsReloading();

		bIsSubSkilling = PlayerChar->IsSubSkilling();

		ViewPitch = PlayerChar->ReplicatedViewPitch;
	}
}


void UPlayerAnimInstance::PlayBaseAttackMontage()
{
	if (!IsValid(BaseAttackMontage))
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

void UPlayerAnimInstance::AnimNotify_ReloadFinish()
{

	

}

void UPlayerAnimInstance::AnimNotify_StartGrenade()
{
	TObjectPtr<APlayerCharacterBase> PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());

	if (IsValid(PlayerChar))
	{
		PlayerChar->HideWeapon();
	}
}

void UPlayerAnimInstance::AnimNotify_EndGrenade()
{
}

void UPlayerAnimInstance::AnimNotify_MakeGrenade()
{
	TObjectPtr<APlayerCharacterBase> PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());

	if (IsValid(PlayerChar))
	{
		PlayerChar->MakeGrenade();
	}
}

void UPlayerAnimInstance::AnimNotify_StartDead()
{
	TObjectPtr<APlayerCharacterBase> PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());

	if (IsValid(PlayerChar))
	{
		PlayerChar->HideWeapon();
	}
	bIsDead = true;
}

void UPlayerAnimInstance::AnimNotify_EndDead()
{
	APlayerCharacterBase* PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());
	if (IsValid(PlayerChar))
	{
		if (PlayerChar)
		{
			PlayerChar->GetWorldTimerManager().SetTimer(DeadTimerHandle, PlayerChar, &APlayerCharacterBase::OnDead, 2.0f, false);
		}
	}

}

void UPlayerAnimInstance::AnimNotify_FootStep()
{
	TObjectPtr<APlayerCharacterBase> PlayerChar = Cast<APlayerCharacterBase>(TryGetPawnOwner());

	if (IsValid(PlayerChar))
	{
		PlayerChar->PlayFootStepSound();
	}
}

void UPlayerAnimInstance::AnimNotify_Reload()
{
	// 총알갈기

}


