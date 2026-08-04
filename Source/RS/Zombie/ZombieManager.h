// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZombieManager.generated.h"

class AZombieBase;
class AZombieSpawnPoint;
class UGameplayEffect;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RS_API UZombieManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZombieManager();

protected:
	//레벨에 있는 모든 스폰 포인트를 담을 배열
	UPROPERTY()
	TArray<TObjectPtr<AZombieSpawnPoint>> SpawnPointArray;

	//레벨에 존재하는 모든 좀비를 담을 배열
	UPROPERTY()
	TArray<TObjectPtr<AZombieBase>> AllZombieArray;

	//레벨에 존재할 수 있는 최대 좀비 수량을 정하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta=(AllowPrivateAccess = "true"))
	int32 MaxZombieCount = 150;

	//좀비를 즉시 죽이기 위한 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UGameplayEffect> InstantKillGE;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	//좀비 생성 시 등록하는 함수
	void ZombieRegister(AZombieBase* Zombie);

	//좀비가 죽을 때 등록 해제하는 함수
	void ZombieUnRegister(AZombieBase* Zombie);

	//배치된 스폰 포인트를 등록하는 함수
	void SpawnPointRegister(AZombieSpawnPoint* SpawnPoint);

	//모든 스폰포인트의 활성화/비활성화를 조절하는 함수 (최대 개체수 조절)
	void SetSpawnPointActivation(bool Activate);

	//모든 좀비를 사망 처리하는 함수 (스테이지 클리어)
	void AllZombieDeath();

	//모든 스테이지를 파괴하는 함수 (스테이지 클리어)
	void AllSpawnPointDestroy();

		
};
