// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieStandardAttributeSet.h"
#include "RS/Zombie/ZombieBase.h"


void UZombieStandardAttributeSet::OnHPChanged(AActor* Instigator)
{
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
