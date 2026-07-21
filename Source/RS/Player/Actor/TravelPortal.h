// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TravelPortal.generated.h"

class UBoxComponent;
class UNiagaraComponent;
class APlayerCharacterBase;

UCLASS()
class RS_API ATravelPortal : public AActor
{
	GENERATED_BODY()

public:
	ATravelPortal();

	void SetTarget(FName InLevelName, const FString& InPackagePath);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> CollisionComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FName TargetLevelName;
	UPROPERTY(Replicated, BlueprintReadOnly)
	FString TargetLevelPackagePath;

	TSet<TWeakObjectPtr<APlayerCharacterBase>> ReadyPlayers;

	UPROPERTY(EditDefaultsOnly, Category = "Portal")
	float DepartureDelay = 3.f;

	FTimerHandle DepartureTimerHandle;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	void CheckAllPlayersReady();

private:
	void ExecuteTravel();
};
