// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAnimInstance.h"
#include "RS/Zombie/ZombieBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/LogCategory.h"

UZombieAnimInstance::UZombieAnimInstance()
{

}

void UZombieAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}
void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UZombieAnimInstance::PlayAttackMontage(int32 Number)
{
	FName Key = *FString::Printf(TEXT("Attack%d"), Number);
	//만든 키로 몽타주를 찾는다.
	UAnimMontage* Montage = *(MontageMap.Find(Key));
	Montage_Play(Montage);
}

void UZombieAnimInstance::AnimNotify_Attack()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Attack();
	}
}

void UZombieAnimInstance::AnimNotify_AttackEnd()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		TObjectPtr<AAIController> Ctrl = Zombie->GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);
	}
}

void UZombieAnimInstance::AnimNotify_Death()
{
	//Death 함수에서는 캡슐의 충돌 제거, 메쉬의 충돌 활성화 등의 작업을 해야 한다.
	//이 작업은 자동으로 동기화가 되지 않아서 클라에서도 설정해줘야 한다.
	//따라서 Death함수는 서버/클라 모두 호출해야 한다.
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (IsValid(Zombie))
	{
		Zombie->Death();
	}
}
