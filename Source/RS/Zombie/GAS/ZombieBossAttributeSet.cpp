// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBossAttributeSet.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/LogCategory.h"

UZombieBossAttributeSet::UZombieBossAttributeSet()
{

}

void UZombieBossAttributeSet::OnHPChanged(AActor* Instigator)
{
	RS_LOG_DEBUG(LogRSBattle, Log, TEXT("좀비가 공격받음"));

	//좀비의 HP변화를 처리하는 함수를 호출한다.
	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
	{
		//GetOwnerActor와 GetAvatarActor의 차이점 체크
		if (auto* Zombie = Cast<AZombieBase>(ASC->GetAvatarActor()))
		{
			Zombie->CallBackHP(Instigator);
		}
	}
}
