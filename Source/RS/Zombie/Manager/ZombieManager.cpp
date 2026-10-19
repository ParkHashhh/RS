// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Zombie/Manager/ZombieManager.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/Zombie/ZombieSpawnPoint.h"
#include "RS/Core/LogCategory.h"
#include "AbilitySystemComponent.h"
#include "RS/GameFramework/GameState/RSGameState.h"

// Sets default values for this component's properties
UZombieManager::UZombieManager()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UZombieManager::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UZombieManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UZombieManager::ZombieRegister(AZombieBase* Zombie)
{
	if (!Zombie)
	{
		return;
	}

	//들어온 좀비 포인터를 좀비 배열에 넣는다.
	AllZombieArray.Add(Zombie);

	//좀비 추가 이후에 만약 MaxCount를 넘어갔다면 스폰 포인트를 전부 비활성화 시킨다.
	if (AllZombieArray.Num() >= MaxZombieCount)
	{
		SetSpawnPointActivation(false);
	}
}

void UZombieManager::ZombieUnRegister(AZombieBase* Zombie)
{
	if (!Zombie)
	{
		return;
	}

	//들어온 좀비 포인터를 배열에서 제거한다.
	//Remove : 제거 후 이후 요소를 전부 앞으로 당김
	//RemoveSwap : 제거 후 마지막 요소를 가져와 빈 공간에 삽입
	//좀 더 부하가 적은 함수를 사용했다.
	AllZombieArray.RemoveSwap(Zombie);

	//좀비가 제거되어 MaxCount 아래로 내려갔다면 다시 스폰 포인트를 활성화 시킨다.
	if (AllZombieArray.Num() < MaxZombieCount)
	{
		SetSpawnPointActivation(true);
	}
}

void UZombieManager::BossRegister(AZombieBoss* Boss)
{
	if (!Boss)
	{
		return;
	}

	ARSGameState* GameState = GetWorld()->GetGameState<ARSGameState>();
	if (GameState)
	{
		GameState->RegistBoss(Boss);
	}
}

void UZombieManager::BossUnRegister(AZombieBoss* Boss)
{
	if (!Boss)
	{
		return;
	}

	ARSGameState* GameState = GetWorld()->GetGameState<ARSGameState>();
	if (GameState)
	{
		GameState->UnRegistBoss(Boss);
	}
}

void UZombieManager::SpawnPointRegister(AZombieSpawnPoint* SpawnPoint)
{
	if (!SpawnPoint)
	{
		return;
	}

	//스폰 포인트를 배열에 등록한다.
	SpawnPointArray.Add(SpawnPoint);
}

void UZombieManager::SetSpawnPointActivation(bool Activate)
{
	//배열을 순회하며 모든 스폰포인트의 스위치를 조정한다.
	for (auto SpawnPoint : SpawnPointArray)
	{
		if (IsValid(SpawnPoint))
		{
			SpawnPoint->SetSpawnActivation(Activate);
		}
	}
}

void UZombieManager::AllZombieDeath()
{
	for (int32 i = AllZombieArray.Num() - 1; i >= 0; --i)
	{
		const AZombieBase* Zombie = AllZombieArray[i];
		if (IsValid(Zombie))
		{
			if (UAbilitySystemComponent* ASC = Zombie->GetAbilitySystemComponent())
			{
				if (InstantKillGE)
				{
					FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
					Context.AddSourceObject(Zombie);

					ASC->ApplyGameplayEffectToSelf(InstantKillGE->GetDefaultObject<UGameplayEffect>(), 1.f, Context);
				}
			}
		}
	}
}

void UZombieManager::AllSpawnPointDestroy()
{
	for (auto SpawnPoint : SpawnPointArray)
	{
		if (IsValid(SpawnPoint))
		{
			SpawnPoint->Destroy();
		}
	}
}

