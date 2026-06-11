// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBossAttributeSet.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/LogCategory.h"

UZombieBossAttributeSet::UZombieBossAttributeSet()
{

}

void UZombieBossAttributeSet::OnHpChanged(AActor* Instigator)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("좀비가 공격받음"));

	//좀비의 HP변화를 처리하는 함수를 호출한다.
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (IsValid(ASC))
	{
		//GetOwnerActor와 GetAvatarActor의 차이점 체크
		AZombieBase* Zombie = Cast<AZombieBase>(ASC->GetAvatarActor());
		if (IsValid(Zombie))
		{
			Zombie->CallBackHP(Instigator);
		}
	}
}
