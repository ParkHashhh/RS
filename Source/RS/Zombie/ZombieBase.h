// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSPawn.h"
#include "../RSConstants.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystemInterface.h"
#include "ZombieBase.generated.h"

class UCapsuleComponent;
class UFloatingPawnMovement;
class UAbilitySystemComponent;
class AZombieAIController;
class UBaseAttributeSet;

UCLASS()
class RS_API AZombieBase : public ARSPawn,
	public IGenericTeamAgentInterface,
	public IAbilitySystemInterface

{
	GENERATED_BODY()

public:
	inline static const FName ZombieCollisionName = TEXT("Zombie");

	// Sets default values for this pawn's properties
	AZombieBase();

protected:
	//필요한 부품 : 충돌체, 메쉬, 무브먼트, GAS, 어트리뷰트셋, 애님 인스턴스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> ASC;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBaseAttributeSet> AttributeSet;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UZombieAnimInstance> AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	TSubclassOf<AZombieAIController> AICtrl;

	FGenericTeamId TeamID;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const
	{
		return ASC;
	}

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID);
	virtual FGenericTeamId GetGenericTeamId() const;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;
};
