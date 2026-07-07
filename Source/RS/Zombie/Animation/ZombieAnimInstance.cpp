// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAnimInstance.h"
#include "RS/Zombie/ZombieBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/LogCategory.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

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
	UAnimMontage* Montage = *(AnimPresetArray[AnimIndex_Backup].MontageMap.Find(Key));
	Montage_Play(Montage);
}

void UZombieAnimInstance::SetIsTurn(bool IsTurn)
{
	bIsTurn = IsTurn;
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
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = Zombie->GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);

		//자신을 대상으로 일반공격 쿨다운 이펙트를 발동시킨다.
		//쿨다운 중임을 나타내는 태그가 지정된 시간동안 좀비에게 부착된다.
		UAbilitySystemComponent* ASC = Zombie->GetAbilitySystemComponent();
		TSubclassOf<UGameplayEffect> Effect = Zombie->GetEffect(TEXT("NAttackCooldown"));
		if (ASC && IsValid(Effect))
		{
			//이미 쿨다운 태그가 있다면 이펙트 적용 과정을 무시한다.
			if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.AttackCooldown"))))
			{
				return;
			}

			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(Zombie);
			ASC->ApplyGameplayEffectToSelf(Effect->GetDefaultObject<UGameplayEffect>(), 1.f, Context);
		}
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

void UZombieAnimInstance::AnimNotify_RoarEnd()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		TObjectPtr<AAIController> Ctrl = Zombie->GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("IsRoar"), false);
	}
}

void UZombieAnimInstance::AnimNotify_RFootStep()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (IsValid(Zombie))
	{
		Zombie->PlayFootStepCue(TEXT("foot_r"));
	}
}

void UZombieAnimInstance::AnimNotify_LFootStep()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (IsValid(Zombie))
	{
		Zombie->PlayFootStepCue(TEXT("foot_l"));
	}
}
