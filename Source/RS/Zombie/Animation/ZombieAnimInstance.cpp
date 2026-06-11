// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAnimInstance.h"
#include "RS/Zombie/ZombieBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

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
