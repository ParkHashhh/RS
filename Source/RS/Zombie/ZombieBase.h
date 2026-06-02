// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSPawn.h"
#include "GenericTeamAgentInterface.h"
#include "ZombieBase.generated.h"

//임시로 사용할 팀 아이디 : 나중에 상위 클래스에 선언되면 제거할 예정이다.
#define	TeamNeutral	2
#define	TeamZombie	1
#define	TeamPlayer	0

UCLASS()
class RS_API AZombieBase : public ARSPawn,
	public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AZombieBase();

protected:
	//필요한 부품 : 충돌체, 메쉬, 무브먼트, GAS, 어트리뷰트셋, 애님 인스턴스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	TObjectPtr<class UCapsuleComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UFloatingPawnMovement> Movement;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UAbilitySystemComponent> ASC;
		
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UZombieAttributeSet> AttributeSet;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//TObjectPtr<class UZombieAnimInstance> AnimInstance;

	FGenericTeamId TeamID;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID);
	virtual FGenericTeamId GetGenericTeamId() const;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;
};
