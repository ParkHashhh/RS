// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawnPoint.generated.h"

class USplineComponent;
class UArrowComponent;

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

	FTimerHandle		SpawnTimer;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector>		FirstDstPointArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty")
	float		SpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty")
	bool		bInfiniteSpawnUseTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty")
	bool		bDefaultSpawn = false;;

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

};
