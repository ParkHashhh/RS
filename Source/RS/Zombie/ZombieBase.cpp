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

// Sets default values
AZombieBase::AZombieBase()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//필요한 부품들을 생성한다.
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	Body->SetCollisionProfileName(ZombieCollisionName);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Body);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Body);

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

	//AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());

	ASC->InitAbilityActorInfo(this, this);
	if (AttackAbility)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(AttackAbility, 1));
	}
	//ASC->GiveAbility(FGameplayAbilitySpec(UGameplayAbility_ZombieAttack::StaticClass(), 1));
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
	AnimInstance = Cast<UZombieAnimInstance>(Mesh->GetAnimInstance());
}

// Called every frame
void AZombieBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	GEngine->AddOnScreenDebugMessage(-1, 30.f, FColor::Red, TEXT("Test"));
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

	//클라이언트의 애님 인스턴스에게 애니메이션 타입 변환 함수를 호출하도록 신호를 보낸다.
	if (IsValid(AnimInstance))
	{
		AnimInstance->SetAnim(AnimType);
	}

}

