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

// Sets default values
AZombieBase::AZombieBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	if (IsValid(AICtrl))
	{
		AIControllerClass = AICtrl;
	}

	if (GetController() == nullptr)
	{
		SpawnDefaultController();
	}

	ASC->InitAbilityActorInfo(this, this);
	if (AttackAbility && HasAuthority())
	{
		ASC->GiveAbility(FGameplayAbilitySpec(AttackAbility, 1));
	}
}

void AZombieBase::PostInitProperties()
{
	Super::PostInitProperties();

	//디테일에서 지정한 값은 이 타이밍에 들어가게 된다.
}

void AZombieBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//Beginplay는 상황에 따라 확실하지 않을 수 있으므로 확실한 타이밍인 이곳에서 멤버 변수에 넣어준다.
	//AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());
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
	//컨트롤러 빙의가 성공했을 때 비헤이비어 트리를 바로 활성화 시켜줘야 한다.
	AZombieAIController* Ctrl = Cast<AZombieAIController>(NewController);
	if (IsValid(Ctrl))
	{
		Ctrl->ActivateBT();
	}
	Super::PossessedBy(NewController);
}

void AZombieBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZombieBase, AnimType);
	DOREPLIFETIME(AZombieBase, bDissolveEnable)
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
	AnimType = Type;

	//현재 데디케이트 서버에서는 필요가 없는 기능이다.
	//하지만 리슨 서버에서는 필요한 기능이므로 남겨둔다.
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

		//공격 신호가 들어왔다면 몽타주를 재생하라고 한 뒤 바로 탈출한다.
		if (Type == EZombieAnimType::Attack)
		{
			AtkMontageIndex = FMath::RandRange(1, AnimInstance->GetMontageNum());
			AnimInstance->PlayAttackMontage(AtkMontageIndex);
			return;
		}

		AnimInstance->SetAnim(Type);
	}
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
	if (!InstigatorActor)
	{
		RS_LOG_DEBUG(LogRSCommon, Error, TEXT("%s: InstigatorActor is nullptr"), *LOG_LINE_INFO);
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

		AAIController* Ctrl = GetController<AAIController>();
		if (IsValid(Ctrl))
		{
			Ctrl->BrainComponent->StopLogic(TEXT("Death"));
			Ctrl->BrainComponent->Cleanup();
		}

		Movement->StopMovementImmediately();
		Movement->Deactivate();
		Movement->SetComponentTickEnabled(false);

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
			if (BB->GetValueAsObject(TEXT("Target")) == nullptr)
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

	//랙돌 처리를 서버에서도 하도록 요청한다.
	Death_Server();
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

void AZombieBase::OnRep_ChangeAnimType()
{
	//서버는 애니메이션을 재생할 필요가 없기 때문이다.
	if (HasAuthority())
	{
		return;
	}

	//액터가 복제될 때 애님 인스턴스를 멤버 변수에 넣지 않은 상태일 수 있으므로 안전장치를 만든다.
	if (!IsValid(AnimInstance))
	{
		AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());
	}

	//애님 타입이 Attack으로 변경되었다면 공격 몽타주를 재생하고 바로 탈출한다.
	if (AnimType == EZombieAnimType::Attack)
	{
		AnimInstance->PlayAttackMontage(AtkMontageIndex);
		return;
	}

	//클라이언트의 애님 인스턴스에게 애니메이션 타입 변환 함수를 호출하도록 신호를 보낸다.
	if (IsValid(AnimInstance))
	{
		AnimInstance->SetAnim(AnimType);
	}
}

bool AZombieBase::Death_Server_Validate()
{
	if (AttributeSet->GetCurrentHP() > 0)
	{
		return false;
	}
	return true;
}

void AZombieBase::Death_Server_Implementation()
{
	//신호 검증은 _Validate 함수에서 수행했다.
	//이곳에서는 중복 연산을 체크한다.
	if (Mesh->bBlendPhysics)
	{
		return;
	}
	//클라와 동일하게 서버에서도 처리하도록 동일한 함수를 호출해준다.
	ApplyRagdoll();
}

