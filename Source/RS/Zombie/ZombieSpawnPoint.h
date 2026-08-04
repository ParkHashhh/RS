// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawnPoint.generated.h"

class USplineComponent;
class UArrowComponent;	
class UBoxComponent;

UCLASS()
class RS_API AZombieSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AZombieSpawnPoint();

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent>	Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent>	FirstDstList;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent>	DetectArea;

#if WITH_EDITORONLY_DATA
	/** Component shown in the editor only to indicate character facing */
	UPROPERTY()
	TObjectPtr<UArrowComponent> ArrowComponent;
#endif

	UPROPERTY(EditAnywhere, Category = "NeedProperty")
	TSubclassOf<class AZombieBase>	SpawnClass;

	UPROPERTY(EditAnywhere, Category = "NeedProperty")
	float			SpawnInterval;

	UPROPERTY(EditAnywhere, Category = "NeedProperty")
	int32			SpawnNumber;

	//무한스폰 타이머 핸들을 보관하기 위한 멤버 변수
	FTimerHandle		SpawnTimer;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector>		FirstDstPointArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty")
	float		SpawnRadius;

	//시간에 기반해 무한 스폰을 하는 스폰 포인트인지를 결정하는 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty")
	bool		bInfiniteSpawnUseTimer;

	//초기 스폰용 스폰 포인트인지를 결정하는 불리언 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty")
	bool		bDefaultSpawn = false;

	//수량에 따라 스폰포인트 활성화 여부를 제어할 불리언 변수
	bool		bSpawnActivateByMaxCount = true;

	//영역 내의 플레이어 수
	int32 PlayerNumInArea = 0;

	//플레이어 존재에 따른 스폰 포인트 활성화 여부를 제어할 불리언 변수
	bool bSpawnActivateByPlayerIn = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform);

private:
	void SpawnTimerFinish();
	void FirstSpawnZombie();
	void SpawnZombie();
	bool CalcRandomSpawnLoc(FVector& Loc);
	bool CalcRandomFirstDst(FVector& Loc);

	UFUNCTION()
	void PlayerInArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void PlayerOutArea(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void AddPlayerCount(AActor* OtherActor);
	void CheckStartOverlap();

public:
	void SpawnZombieByBossTrigger(AActor* Target);
	void SetSpawnActivation(bool IsActivate);

};
