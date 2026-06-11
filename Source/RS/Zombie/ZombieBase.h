// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSPawn.h"
#include "../RSConstants.h"
#include "RS/RSTypes.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystemInterface.h"
#include "ZombieBase.generated.h"

class UCapsuleComponent;
class UFloatingPawnMovement;
class UAbilitySystemComponent;
class AZombieAIController;
class UBaseAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UZombieAnimInstance;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZombieAnimInstance> AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	TSubclassOf<AZombieAIController> AICtrl;

	FGenericTeamId TeamID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"));
	TSubclassOf<UGameplayAbility> AttackAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSubclassOf<UGameplayEffect>> EffectMap;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ChangeAnimType)
	EZombieAnimType AnimType;

	UPROPERTY();
	FVector SpawnPointLocation;
	UPROPERTY();
	FVector NextPointLocation;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;
	virtual void PostInitializeComponents() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void SetSpawnPointLocation(FVector Loc)
	{
		SpawnPointLocation = Loc;
	}

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const
	{
		return ASC;
	}
	UBaseAttributeSet* GetAttributeSet() const
	{
		return AttributeSet;
	}
	FVector GetNextPointLocation() const
	{
		return NextPointLocation;
	}

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID);
	virtual FGenericTeamId GetGenericTeamId() const;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;
public:
	virtual void Attack();
	TSubclassOf<UGameplayEffect> GetEffect(const FName& Name);
	void ChangeAnim(EZombieAnimType Type);
	void CalcNextPointLocation();

public:
	UFUNCTION()
	void OnRep_ChangeAnimType();
};
