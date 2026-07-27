// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieBoss_Behemoth_PickupPoint.generated.h"

class USplineComponent;
class UArrowComponent;

UCLASS()
class RS_API AZombieBoss_Behemoth_PickupPoint : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent>	Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent>	PickupPoint;

	UPROPERTY(VisibleAnywhere)
	TArray<FVector>		PickupPointArray;

#if WITH_EDITORONLY_DATA
	/** Component shown in the editor only to indicate character facing */
	UPROPERTY()
	TObjectPtr<UArrowComponent> ArrowComponent;
#endif
	
public:	
	// Sets default values for this actor's properties
	AZombieBoss_Behemoth_PickupPoint();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnConstruction(const FTransform& Transform);

	void CopyPickupPointArray(TArray<FVector>& Array);

};
