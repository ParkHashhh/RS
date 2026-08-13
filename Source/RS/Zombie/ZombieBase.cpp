// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "AbilitySystemComponent.h"
#include "AI/ZombieAIController.h"
#include "GAS/Ability/GameplayAbility_ZombieAttack.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "Animation/ZombieAnimInstance.h"
#include "NavigationSystem.h"
#include "RS/GAS/BaseAttributeSet.h"
#include "BrainComponent.h"
#include "RS/LogCategory.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RS/Data/Subsystem/DataSubsystem.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/GameMode/MainGameMode.h"
#include "RS/Zombie/ZombieManager.h"

// Sets default values
AZombieBase::AZombieBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));

	bUseControllerRotationYaw = true;

	//객체별 AttributeSet의 타입이 다르므로 AttributeSet의 생성, 연결은 자식 객체에서 진행해야 한다.
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));

	ASC->SetIsReplicated(true);
	SetReplicates(true);
	SetReplicateMovement(true);

	SetGenericTeamId(FGenericTeamId(TeamMonster));

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AZombieBase::BeginPlay()
{
	Super::BeginPlay();

	const UGameInstance* Inst = GetGameInstance();
	check(Inst);

	UDataSubsystem* DataSubSystem = Inst->GetSubsystem<UDataSubsystem>();
	check(DataSubSystem);

	DataSubSystem->ExecuteOrBindOnZombieDefaultInfoLoaded(this, &AZombieBase::LoadZombieDefaultData);

	if (GetController() == nullptr)
	{
		SpawnDefaultController();
	}

	if (HasAuthority())
	{
		InitAbility();
	}
}

void AZombieBase::PostInitProperties()
{
	Super::PostInitProperties();
}

void AZombieBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (IsValid(Mesh))
	{
		AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());
	}

	if (Movement && Body)
	{
		Movement->SetUpdatedComponent(Body);
	}
}

// Called every frame
void AZombieBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//디졸브 수치를 변경한다.
	if (bDissolveEnable)
	{
		//디졸브가 지정된 시간동안 총 1이 감소되어야 한다.
		Dissolve -= DeltaTime / DissolveTime;

		//디졸브 연출이 끝나면 서버에서 액터를 Destroy 시킨다.
		if (Dissolve <= -0.5f && HasAuthority())
		{
			Destroy();
		}

		for (auto& Mtrl : MaterialArray)
		{
			Mtrl->SetScalarParameterValue(TEXT("Dissolve"), Dissolve);
		}
	}
}

// Called to bind functionality to input
void AZombieBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AZombieBase::PossessedBy(AController* NewController)
{
	//컨트롤러 빙의 후 바로 비헤이비어 트리를 가동한다.
	AZombieAIController* Ctrl = Cast<AZombieAIController>(NewController);
	if (IsValid(Ctrl))
	{
		AICtrl = Ctrl;

		Ctrl->ActivateBT();
	}
	Super::PossessedBy(NewController);
}

void AZombieBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZombieBase, AnimType)
	DOREPLIFETIME(AZombieBase, bDissolveEnable)
	DOREPLIFETIME(AZombieBase, AnimIndex)
	DOREPLIFETIME(AZombieBase, AtkMontageNumber)
	DOREPLIFETIME(AZombieBase, SkillMontageNumber)
	DOREPLIFETIME(AZombieBase, bIsJumpEnd)
	DOREPLIFETIME(AZombieBase, PrevAnimType)
	DOREPLIFETIME(AZombieBase, bIsBoss)
}

void AZombieBase::SetIsJumpEnd(bool IsEnd)
{
	bIsJumpEnd = IsEnd;

	if (HasAuthority() && bIsJumpEnd)
	{
		AnimInstance->FinishJump();
	}
}

void AZombieBase::SetGenericTeamId(const FGenericTeamId& ID)
{
	TeamID = ID;
}

FGenericTeamId AZombieBase::GetGenericTeamId() const
{
	return TeamID;
}

ETeamAttitude::Type AZombieBase::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(&Other);

	if (!OtherTeamAgent)
		return ETeamAttitude::Neutral;

	else if (OtherTeamAgent->GetGenericTeamId().GetId() == TeamNeutral)
		return ETeamAttitude::Neutral;

	return GetGenericTeamId() == OtherTeamAgent->GetGenericTeamId() ?
		ETeamAttitude::Friendly : ETeamAttitude::Hostile;
}

void AZombieBase::Attack()
{
	AAIController* Ctrl = GetController<AAIController>();
	if (IsValid(Ctrl))
	{
		//좀비의 공격 캡슐을 생성할 위치와 각도를 정한다.
		TArray<FHitResult> HitArray;
		FVector StartLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
		FVector EndLoc = StartLoc + GetActorForwardVector() * 50.f;
		FQuat CapsuleRot = FQuat::FindBetweenNormals(FVector::UpVector,
			GetActorRightVector());

		//나 자신은 무시하도록 파라미터를 만든다.
		FCollisionQueryParams Param(NAME_None, false, this);

		//캡슐 모양으로 임시 충돌체를 생성해서 범위 충돌을 체크한다.
		bool Collision = GetWorld()->SweepMultiByChannel(HitArray, StartLoc, EndLoc, CapsuleRot, ECollisionChannel::ECC_GameTraceChannel6,
			FCollisionShape::MakeCapsule(20.f, 20.f), Param);

		FColor DebugColor = FColor::Green;

		//SweepMultiByChannel 함수는 블록 충돌판정이 나와야 true를 반환한다.
		//클리빙 어택 허용 : 현재 코드 유지
		//클리빙 어택 금지 : 트레이스와 플레이어가 block이 되도록 하고, 조건을 Collision이 true 인지 체크한다.
		if (HitArray.Num() > 0)
		{
			for (auto Hit : HitArray)
			{
				AActor* HitActor = Hit.GetActor();
				if (!HitActor)
				{
					continue;
				}

				DebugColor = FColor::Red;

				FGameplayEventData EventData;

				EventData.Target = HitActor;
				EventData.Instigator = this;
				EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.Attack"));

				FGameplayAbilityTargetData_SingleTargetHit* TargetData =
					new FGameplayAbilityTargetData_SingleTargetHit(Hit);

				EventData.TargetData.Add(TargetData);

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
			}
		}

#if WITH_EDITOR

		DrawDebugCapsule(GetWorld(), (StartLoc + EndLoc) * 0.5f, 20.f, 20.f, CapsuleRot,
			DebugColor, false, 1.f);

#endif

	}
}

TSubclassOf<UGameplayEffect> AZombieBase::GetEffect(const FName& Name)
{
	TSubclassOf<UGameplayEffect>* FindResult = EffectMap.Find(Name);
	if (!FindResult)
	{
		return nullptr;
	}
	return *FindResult;
}
void AZombieBase::ChangeAnim(EZombieAnimType Type)
{
	if (AnimType == EZombieAnimType::Death)
	{
		return;
	}

	PrevAnimType = AnimType;
	AnimType = Type;

	if (HasAuthority())
	{
		//애님 인스턴스를 제대로 못 가져올 경우를 대비해 한번 더 가져오도록 한다.
		if (!IsValid(AnimInstance))
		{
			AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());
		}
		if (!IsValid(AnimInstance))
		{
			return;
		}

		if (Type == EZombieAnimType::Idle ||
			Type == EZombieAnimType::CombatIdle ||
			Type == EZombieAnimType::Walk ||
			Type == EZombieAnimType::Run ||
			Type == EZombieAnimType::Roar ||
			Type == EZombieAnimType::Death)
		{
			if (Type == EZombieAnimType::Walk)
			{
				ChangeSpeed_Walk();
			}
			else if (Type == EZombieAnimType::Run)
			{
				ChangeSpeed_Run();
			}

			AnimInstance->SetAnim(Type);
		}
		else
		{
			UseAnimMontage(Type, PrevAnimType);
		}
	}
}

void AZombieBase::UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType)
{
	//일반 공격은 모든 좀비의 공통 기능이므로 부모에서 호출한다.
	//공격 신호가 들어왔다면 몽타주를 재생하라고 한 뒤 바로 탈출한다.
	if (Type == EZombieAnimType::Attack)
	{
		if (HasAuthority())
		{
			AtkMontageNumber = FMath::RandRange(1, AnimInstance->GetAtkMontageNum());
		}
		AnimInstance->PlayAttackMontage(AtkMontageNumber);
		//좀비가 공격을 한다면 기저에 재생되고 있는 애니메이션을 전투 대기로 바꿔준다.
		AnimInstance->SetAnim(EZombieAnimType::CombatIdle);
	}

	if (Type == EZombieAnimType::Jump)
	{
		AnimInstance->PlayCommonMontage(TEXT("Jump"));
		//점프 직전 어떤 애니메이션을 재생 중이였는지를 확인하고 맞는 애니메이션으로 연결한다.
		if (BeforeType == EZombieAnimType::Walk)
		{
			AnimInstance->SetAnim(EZombieAnimType::Walk);
		}
		else if (BeforeType == EZombieAnimType::Run)
		{
			AnimInstance->SetAnim(EZombieAnimType::Run);
		}
		
	}
}

bool AZombieBase::CheckRoar()
{
	return AnimInstance->ChechRoarAnimValid();;
}

void AZombieBase::CalcNextPointLocation()
{
	//내비 메시 볼륨은 서버에만 있으므로 클라에서는 동작하지 못하도록 해야 한다.
	if (!HasAuthority())
	{
		return;
	}

	//이동 가능한 무작위 위치를 만들어주는 함수를 호출하기 위해 네비게이션 시스템을 얻어온다.
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());

	//생성된 위치를 담을 구조체
	FNavLocation ResultLoc;

	//무작위 좌표를 만들 범위
	float Radius = 600.f;

	//성공했다면 만든 좌표를, 실패했다면 현재 위치를 다음 이동위치로 지정한다.
	bool Result = NavSystem->GetRandomReachablePointInRadius(SpawnPointLocation, Radius, ResultLoc);
	if (Result)
	{
		NextPointLocation = ResultLoc.Location;
	}
	else
	{
		NextPointLocation = GetActorLocation();	
	}
}

void AZombieBase::CallBackHP(AActor* InstigatorActor)
{
	if (!HasAuthority())
	{
		return;
	}

	if (AttributeSet->GetCurrentHP() <= 0)
	{
		//좀비의 사망 애니메이션을 동작시킨다
		//이 함수를 호출해야 클라이언트에서도 애니메이션을 사망 애니메이션으로 변경한다.
		ChangeAnim(EZombieAnimType::Death);

		//사망 처리 시 서버에서 해야 할 일 - 판정과 관련된 정보 정리
		//1. 인공지능 종료
		//2. 무브먼트 종료
		//3. 캡슐의 충돌 제거
		//4. 좀비 매니저에서 해당 좀비 제거

		AAIController* Ctrl = GetController<AAIController>();
		if (IsValid(Ctrl))
		{
			Ctrl->BrainComponent->StopLogic(TEXT("Death"));
			Ctrl->BrainComponent->Cleanup();
		}

		Movement->StopMovementImmediately();
		Movement->Deactivate();
		Movement->SetComponentTickEnabled(false);

		//좀비 매니저를 얻어온 후 나 자신을 제거하도록 함수의 인자로 넘겨준다.
		AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
		if (GameMode)
		{
			UZombieManager* ZombieMgr = GameMode->GetZombieManager();
			ZombieMgr->ZombieUnRegister(this);
		}

		//디졸브 기능 활성화
		bDissolveEnable = true;

		//좀비가 죽어서 플레이어의 GAS를 변화시켜야 한다면 (골드 증가 등)
		//서버에서 해야 할 작업이므로 이곳에서 처리한다.
	}
	else
	{
		//사거리 밖에서 날 공격한 경우 타겟으로 넣는다.
		//이때 기존에 추적중이던 타겟이 없어야 한다.
		AAIController* Ctrl = GetController<AAIController>();
		UBlackboardComponent* BB = Ctrl->GetBlackboardComponent();
		if (Ctrl && BB)
		{
			APlayerCharacterBase* CastedTarget = Cast<APlayerCharacterBase>(InstigatorActor);
			//이때도 타겟이 죽은 타겟이 아니여야 한다.
			if (BB->GetValueAsObject(TEXT("Target")) == nullptr && !CastedTarget->IsDead())
			{
				Ctrl->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), InstigatorActor);
			}
		}
	}
}

void AZombieBase::Death()
{
	//랙돌 처리를 위한 함수
	ApplyRagdoll();

	//클라에서는 디졸브를 위한 머티리얼 연산을 해야 하므로 파라미터를 ON으로 바꿔준다.
	for (auto& Mtrl : MaterialArray)
	{
		Mtrl->SetScalarParameterValue(TEXT("DissolveEnable"), 1.f);
	}
}

void AZombieBase::ApplyRagdoll()
{
	//서버와 클라이언트 두 곳 모두에서 설정해줘야 하는 것들을 이곳에 넣는다.
	//1. 캡슐 충돌판정 제거
	//2. 메쉬 충돌판정 활성화
	//3. 메쉬 콜리전 프리셋을 랙돌로 변경
	//4. 메쉬의 시뮬레이트 피직스 활성화
	//5. 메쉬의 부분 시뮬레이트 연산
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	Mesh->SetCollisionProfileName(TEXT("Ragdoll"));

	Mesh->SetSimulatePhysics(true);
	//바디 피직스 시뮬레이트 연출 계산에 방해가 될 외부 수치를 제거한다.
	Mesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	Mesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

	//전신 바디 시뮬레이트를 켜되 루트 본은 제외한다.
	Mesh->SetAllBodiesSimulatePhysics(false);
	Mesh->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, true);

	//사용하지 않아 비활성화 된 바디를 강제로 깨워준다.
	Mesh->WakeAllRigidBodies();

	//기존 애니메이션과 시뮬레이트 피직스 결과를 섞도록 한다.
	Mesh->bBlendPhysics = true;
}

void AZombieBase::ChangeSpeed_Walk()
{
	Movement->MaxSpeed = AttributeSet->GetWalkSpeed();
}

void AZombieBase::ChangeSpeed_Run()
{
	Movement->MaxSpeed= AttributeSet->GetRunSpeed();
}

float AZombieBase::CalcDistanceToTarget(AActor* Target)
{
	FVector TargetLoc = Target->GetActorLocation();
	FVector ZombieLoc;

	//타겟과 가장 가까운 좀비의 셰이프 위의 좌표를 구한다.
	Body->GetClosestPointOnCollision(TargetLoc, ZombieLoc);

	//두 좌표의 Z값을 일치시킨다 (계산에 영향을 주지 않도록 한다)
	TargetLoc.Z = ZombieLoc.Z;

	float Distance = FVector::Dist(TargetLoc, ZombieLoc);

	return Distance;
}

void AZombieBase::PlayFootStepCue(const FName& FootName)
{
	//발바닥 위치와 라인트레이스를 발사할 위치를 구한다.
	FVector FootLoc = Mesh->GetSocketLocation(FootName);
	FootLoc.Z += 20.f;
	FVector EndLoc = FootLoc - FVector(0.0, 0.0, 500.0);

	FHitResult Result;
	FCollisionQueryParams Params;
	//피지컬 머티리얼을 히트 구조체에 담기 위한 설정
	Params.bReturnPhysicalMaterial = true;
	Params.bTraceComplex = true;
	//발 소켓에서 발사한 라인이 내 발바닥 표면에 충돌하는 것을 막는다.
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Result, FootLoc, EndLoc, ECC_Visibility, Params))
	{
		//피지컬 머티리얼을 얻어온다.
		UPhysicalMaterial* PMtrl = Result.PhysMaterial.Get();

		if (IsValid(PMtrl))
		{
			EPhysicalSurface Surface = PMtrl->SurfaceType;

			//맵에서 꺼내온 사운드 큐 포인터가 유효한지 확인 : 머티리얼은 있는데 Surface 지정을 안한 경우도 있으므로
			const TObjectPtr<USoundBase>* SoundCuePtr = FootStepSoundCueMap.Find(Surface);

			if (!SoundCuePtr)
			{
				return;
			}

			USoundBase* SoundCue = *SoundCuePtr;

			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundCue, Result.ImpactPoint, 0.5f);
		}
	}
}

void AZombieBase::InitAbility()
{
	ASC->InitAbilityActorInfo(this, this);

	if (AttackAbility)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(AttackAbility, 1));
	}
}

float AZombieBase::GetHalfHeight()
{
	return 0.0f;
}

void AZombieBase::PlayZombieAttackSound()
{
	TObjectPtr<USoundBase>* SoundPtr = SoundCueMap.Find(TEXT("Attack"));
	if (SoundPtr)
	{
		USoundBase* Sound = *SoundPtr;
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
}

void AZombieBase::PlayZombieSkillSound()
{
	TObjectPtr<USoundBase>* SoundPtr = SoundCueMap.Find(TEXT("Skill"));
	if (SoundPtr)
	{
		USoundBase* Sound = *SoundPtr;
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
}

void AZombieBase::SpawnZombieAttackParticle()
{
	TObjectPtr<UParticleSystem>* ParticlePtr = ParticleMap.Find(TEXT("Attack"));
	if (ParticlePtr)
	{
		UParticleSystem* Particle = *ParticlePtr;
		FVector ForwardDir = GetActorForwardVector();
		FVector RightDir = GetActorRightVector();
		FVector SpawnLoc = GetActorLocation() - (ForwardDir * 40.f) - (RightDir * 40.f) - FVector(0.0, 0.0, 160.0);
		FRotator ActorRot = GetActorRotation();
		FRotator SpawnRot = ActorRot - FRotator(0.0, 90.0, 0.0);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, SpawnLoc, SpawnRot);
	}

	TObjectPtr<UNiagaraSystem>* NiagaraPtr = NiagaraParticleMap.Find(TEXT("Attack"));
	if (NiagaraPtr)
	{
		UNiagaraSystem* Niagara = *NiagaraPtr;
		FVector ForwardDir = GetActorForwardVector();
		FVector RightDir = GetActorRightVector();
		FVector SpawnLoc = GetActorLocation() - (ForwardDir * 40.f) - (RightDir * 40.f) - FVector(0.0, 0.0, 160.0);
		FRotator ActorRot = GetActorRotation();
		FRotator SpawnRot = ActorRot - FRotator(0.0, 90.0, 0.0);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Niagara, SpawnLoc, SpawnRot, FVector(0.3, 0.3, 0.3));
	}
}

void AZombieBase::SpawnZombieSkillParticle()
{
	TObjectPtr<UParticleSystem>* ParticlePtr = ParticleMap.Find(TEXT("Skill"));
	TObjectPtr<UNiagaraSystem>* NiagaraPtr = NiagaraParticleMap.Find(TEXT("Skill"));
	if (ParticlePtr)
	{
		UParticleSystem* Particle = *ParticlePtr;
		FVector SpawnLoc = GetActorLocation();;
		FRotator SpawnRot = GetActorRotation();
		FVector Scale = FVector(2.f, 2.f, 2.f);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, SpawnLoc, SpawnRot, Scale);
	}
	else if (NiagaraPtr)
	{
		UNiagaraSystem* Niagara = *NiagaraPtr;
		FVector SpawnLoc = GetActorLocation();
		FRotator SpawnRot = GetActorRotation();
		FVector Scale = FVector(2.f, 2.f, 2.f);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Niagara, SpawnLoc, SpawnRot, Scale);
	}
}

void AZombieBase::PlayZombieRoarSound()
{
	TObjectPtr<USoundBase>* SoundPtr = SoundCueMap.Find(TEXT("Roar"));
	if (SoundPtr)
	{
		USoundBase* Sound = *SoundPtr;
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
}

void AZombieBase::OnRep_ChangeAnimType()
{
	//서버는 이미 ChangeAnim에서 처리했다.
	if (HasAuthority())
	{
		return;
	}

	//애님 인스턴스를 제대로 못 가져올 경우를 대비해 한번 더 가져오도록 한다.
	if (!IsValid(AnimInstance))
	{
		AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());
	}

	if (!IsValid(AnimInstance))
	{
		return;
	}

	if (AnimType == EZombieAnimType::Idle ||
		AnimType == EZombieAnimType::CombatIdle ||
		AnimType == EZombieAnimType::Walk ||
		AnimType == EZombieAnimType::Run ||
		AnimType == EZombieAnimType::Roar ||
		AnimType == EZombieAnimType::Death)
	{
		if (AnimType == EZombieAnimType::Walk)
		{
			ChangeSpeed_Walk();
		}
		else if (AnimType == EZombieAnimType::Run)
		{
			ChangeSpeed_Run();
		}

		AnimInstance->SetAnim(AnimType);
	}
	else
	{
		UseAnimMontage(AnimType, PrevAnimType);
	}
}

void AZombieBase::OnRep_ChangeAnimSet()
{
	UZombieAnimInstance* AnimInst = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());
	if (!IsValid(AnimInst))
	{
		return;
	}
	
	AnimInstance = AnimInst;

	AnimInstance->SetAnimPresetIndexBackup(AnimIndex);

	//블루프린트의 변수에 스피드를 넣어주어야 하므로 커스텀 이벤트를 호출한다.
	FName FuncName = FName(TEXT("SetNativeSpeed"));
	UFunction* Func = AnimInstance->FindFunction(FuncName);
	if (Func)
	{
		//인자는 필요 없으므로 nullptr을 넣는다.
		AnimInstance->ProcessEvent(Func, nullptr);
	}
}

void AZombieBase::OnRep_ChangeJumpMontageSection()
{
	//값이 true로 변했을 때에만 몽타주 섹션 변경을 수행한다.
	if (bIsJumpEnd)
	{
		AnimInstance->FinishJump();
	}
}

void AZombieBase::LoadZombieDefaultData()
{
	const UGameInstance* Inst = GetGameInstance();
	check(Inst);

	UDataSubsystem* DataSubsystem = Inst->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	const FZombieDefaultInfo* ZombieDefaultInfo = DataSubsystem->GetZombieDefaultTableInfo(ZombieName);
	if (!ZombieDefaultInfo)
	{
		RS_LOG_DEBUG(LogRSData, Error, TEXT("%s: No valid zombie default data"), *LOG_LINE_INFO);
		return;
	}

	InitStatData(ZombieDefaultInfo);
}

void AZombieBase::InitStatData(const FZombieDefaultInfo* Data)
{

}

