// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/RSPawn.h"
#include "../RSConstants.h"
#include "RS/RSTypes.h"
#include "GenericTeamAgentInterface.h"
#include "AbilitySystemInterface.h"
#include "RS/Utils/KillableInterface.h"
#include "ZombieBase.generated.h"

class UCapsuleComponent;
class UFloatingPawnMovement;
class UAbilitySystemComponent;
class AZombieAIController;
class UBaseAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
class UZombieAnimInstance;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FZombieSkeletalMeshSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> BodyMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USkeletalMesh>> ClothMeshArray{};
};

UCLASS()
class RS_API AZombieBase : public ARSPawn,
                           public IGenericTeamAgentInterface,
                           public IAbilitySystemInterface,
                           public IKillableInterface

{
	GENERATED_BODY()

public:
	inline static const FName ZombieBodyCollisionName = TEXT("Zombie");
	inline static const FName ZombieMeshCollisionName = TEXT("ZombieBody");

	// Sets default values for this pawn's properties
	AZombieBase();

protected:
	//필요한 부품 : 충돌체, 메쉬, 무브먼트, GAS, 어트리뷰트셋, 애님 인스턴스
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta =(AllowPrivateAccess = "true"))
	TObjectPtr<UShapeComponent> Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<USkeletalMeshComponent>> ClothMeshCompArray;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFloatingPawnMovement> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBaseAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZombieAnimInstance> AnimInstance;

	UPROPERTY()
	TObjectPtr<AZombieAIController> AICtrl;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"));
	TSubclassOf<UGameplayAbility> AttackAbility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	TMap<FName, TSubclassOf<UGameplayEffect>> EffectMap;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ChangeAnimType)
	EZombieAnimType AnimType;

	//점프 애니메이션에서 점프 후 걷기/뛰기 선택을 위한 백업 변수
	UPROPERTY(Replicated)
	EZombieAnimType PrevAnimType;

	//배회 기준점을 넣는 멤버 변수
	UPROPERTY();
	FVector SpawnPointLocation;

	//배회에서 다음 지점을 미리 계산해 놓는 멤버변수
	UPROPERTY();
	FVector NextPointLocation;

	//애니메이션 일치를 위한 리플리케이트 애님몽타주 인덱스 멤버변수
	UPROPERTY(Replicated)
	int32 AtkMontageNumber = 1;

	UPROPERTY(Replicated)
	int32 SkillMontageNumber = 1;

	FGenericTeamId TeamID;

	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialArray;

	//디졸브는 0.5 시작, -0.5 완료임을 테스트로 확인함
	float Dissolve = 0.5f;
	float DissolveTime = 3.f;
	UPROPERTY(Replicated)
	bool bDissolveEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NeedProperty", meta=(AllowPirvateAccess = "true"))
	float TurnSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TMap<TEnumAsByte<EPhysicalSurface>, TObjectPtr<USoundBase>> FootStepSoundCueMap;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<USoundBase>> SoundCueMap;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UParticleSystem>> ParticleMap;

	UPROPERTY(EditDefaultsOnly, Category = "NeedProperty", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TMap<FName, TObjectPtr<UNiagaraSystem>> NiagaraParticleMap;

	//행 이름으로써 검색을 위해 좀비이름을 들고 있게 한다. = TableRowName
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NeedProperty", meta = (AllowPrivateAccess = "true"))
	FName ZombieName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_ChangeAnimSet)
	int32 AnimIndex = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ChangeJumpMontageSection)
	bool bIsJumpEnd = false;

	//보스인지 확인하기 위한 변수
	UPROPERTY(Replicated);
	bool bIsBoss;

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
	EZombieAnimType GetAnimType() const
	{
		return AnimType;
	}
	void SetSpawnPointLocation(FVector Loc)
	{
		SpawnPointLocation = Loc;
	}

	void SetIsJumpEnd(bool IsEnd);
	void SetIsBoss(bool IsBoos)
	{
		bIsBoss = IsBoos;
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

	float GetTurnSpeed() const
	{
		return TurnSpeed;
	}

	UZombieAnimInstance* GetAnimInst() const
	{
		return AnimInstance;
	}

	AZombieAIController* GetAICtrl() const
	{
		return AICtrl;
	}

	UFloatingPawnMovement* GetMovement() const
	{
		return Movement;
	}
	bool GetIsBoss() const
	{
		return bIsBoss;
	}

	FName GetTableRowName() const
	{
		return ZombieName;
	}

public:
	virtual void SetGenericTeamId(const FGenericTeamId& ID);
	virtual FGenericTeamId GetGenericTeamId() const;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;

public:
	virtual void Attack();
	TSubclassOf<UGameplayEffect> GetEffect(const FName& Name);
	void ChangeAnim(EZombieAnimType Type);
	virtual void UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType);
	bool CheckRoar();
	void CalcNextPointLocation();
	virtual void CallBackHP(AActor* InstigatorActor);
	virtual void Death();
	//랙돌을 위한 설정을 모아놓은 함수
	void ApplyRagdoll();
	void ChangeSpeed_Walk();
	void ChangeSpeed_Run();
	virtual float CalcDistanceToTarget(AActor* Target);
	void PlayFootStepCue(const FName& FootName);
	virtual void InitAbility();
	virtual float GetHalfHeight();
	virtual void PlayZombieAttackSound();
	virtual void PlayZombieSkillSound();
	virtual void SpawnZombieAttackParticle();
	virtual void SpawnZombieSkillParticle();
	virtual void PlayZombieRoarSound();

public:
	UFUNCTION()
	void OnRep_ChangeAnimType();

	UFUNCTION()
	void OnRep_ChangeAnimSet();

	UFUNCTION()
	void OnRep_ChangeJumpMontageSection();

protected:
	UFUNCTION()
	void LoadZombieDefaultData();
	virtual void InitStatData(const FZombieDefaultInfo* Data);
};
