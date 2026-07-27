// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBoss.h"
#include "GAS/ZombieBossAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RS/LogCategory.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "RS/Zombie/Animation/ZombieAnimInstance.h"
#include "RS/Player/PlayerCharacterBase.h"


// Sets default values
AZombieBoss::AZombieBoss()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AttributeSet = CreateDefaultSubobject<UZombieBossAttributeSet>(TEXT("Attribute"));
	UZombieBossAttributeSet* BossAttri = Cast<UZombieBossAttributeSet>(AttributeSet);
	BossAttributeSet = BossAttri;
	if (IsValid(BossAttri))
	{
		ASC->AddAttributeSetSubobject<UZombieBossAttributeSet>(BossAttri);
	}
}

// Called when the game starts or when spawned
void AZombieBoss::BeginPlay()
{
	Super::BeginPlay();

	BossAttributeSet = Cast<UZombieBossAttributeSet>(AttributeSet);

	if (HasAuthority())
	{
		if (IsValid(AnimInstance) && !AnimInstance->IsAnimPresetArrayEmpty())
		{
			//서버에서 앞으로 사용할 애님 프리셋 인덱스를 추첨한다.
			AnimIndex = FMath::RandRange(0, AnimInstance->GetAnimPresetNum() - 1);

			//호스트도 애님 인스턴스에 인덱스 백업을 넣어둔다.
			OnRep_ChangeAnimSet();
		}

		//타겟 변경 함수를 호출하기 위한 타이머를 세팅한다.
		GetWorld()->GetTimerManager().SetTimer(UpdateTargetTimerHandle,
			FTimerDelegate::CreateUObject(this, &AZombieBoss::ChangeTarget), UpdateTargetInterval, true);
	}

	//바디 메쉬의 머티리얼을 다이나믹 MI로 전환한다.
	int32 MaterialCount = Mesh->GetNumMaterials();
	for (int32 i = 0; i < MaterialCount; ++i)
	{
		UMaterialInstanceDynamic* Mtrl = Mesh->CreateDynamicMaterialInstance(i);
		MaterialArray.Add(Mtrl);
	}

	//시작 지점을 미리 보관하고, 다음 이동 지점을 하나 만들어둔다.
	SpawnPointLocation = GetActorLocation();
	CalcNextPointLocation();

	//클라는 애님 인스턴스 연결이 불완전해 위에서 return되었을 수 있으므로 이곳에서 한번 더 호출한다.
	//이 함수 안에 기준속도를 넣어주는 이벤트 함수가 들어있기 때문이다.
	if (!HasAuthority())
	{
		OnRep_ChangeAnimSet();
	}

	//초기 스탯 지정이 완료되면 이동속도, 감지거리를 넣어준다.
	ChangeSpeed_Run();
}

void AZombieBoss::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

// Called every frame
void AZombieBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZombieBoss::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieBoss, bIsOnlyUpperPlay)
}

void AZombieBoss::Attack()
{
	Super::Attack();
}

void AZombieBoss::RangeAttack()
{

}

void AZombieBoss::RangeAttackEnd()
{

}

void AZombieBoss::InitStatData(const FZombieDefaultInfo* Data)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ASC || !BossAttributeSet)
	{
		RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: ASC or AttributeSet is nullptr"), *LOG_LINE_INFO);
		return;
	}

	//보스 어트리뷰트셋에 데이터를 넣어준다.
	BossAttributeSet->SetAttackPower(Data->AttackPower);
	BossAttributeSet->SetDefensePower(Data->DefensePower);
	BossAttributeSet->SetAttackDistance(Data->AttackDistance);
	BossAttributeSet->SetWalkSpeed(Data->WalkSpeed);
	BossAttributeSet->SetRunSpeed(Data->RunSpeed);
	BossAttributeSet->SetMaxHP(Data->MaxHP);
	BossAttributeSet->SetCurrentHP(Data->MaxHP);
	BossAttributeSet->SetAttackPower(Data->AttackPower);

	BossAttributeSet->SetDetectRange_Sight(Data->DetectRange_Sight);
	BossAttributeSet->SetDetectRange_Hearing(Data->DetectRange_Hearing);

	BossAttributeSet->SetBoss_Skill1Damage(Data->Boss_Skill1Damage);
	BossAttributeSet->SetBoss_Skill1Cooldown(Data->Boss_Skill1Cooldown);
	BossAttributeSet->SetBoss_Skill2Damage(Data->Boss_Skill2Damage);
	BossAttributeSet->SetBoss_Skill2Cooldown(Data->Boss_Skill2Cooldown);
	BossAttributeSet->SetBoss_Skill3Damage(Data->Boss_Skill3Damage);
	BossAttributeSet->SetBoss_Skill3Cooldown(Data->Boss_Skill3Cooldown);
	BossAttributeSet->SetBoss_Skill4Damage(Data->Boss_Skill4Damage);
	BossAttributeSet->SetBoss_Skill4Cooldown(Data->Boss_Skill4Cooldown);
	BossAttributeSet->SetBoss_Skill5Damage(Data->Boss_Skill5Damage);
	BossAttributeSet->SetBoss_Skill5Cooldown(Data->Boss_Skill5Cooldown);
	BossAttributeSet->SetBoss_Skill6Damage(Data->Boss_Skill6Damage);
	BossAttributeSet->SetBoss_Skill6Cooldown(Data->Boss_Skill6Cooldown);

	BossAttributeSet->SetBoss_MeleeAttackDistance(Data->Boss_MeleeAttackDistance);
	BossAttributeSet->SetBoss_MiddleAttackDistance(Data->Boss_MiddleAttackDistance);
	BossAttributeSet->SetBoss_RangeAttackDistance(Data->Boss_RangeAttackDistance);

	TObjectPtr<AZombieAIController> Ctrl = GetController<AZombieAIController>();
	if (IsValid(Ctrl))
	{
		Ctrl->SetDetectRange_Sight(BossAttributeSet->GetDetectRange_Sight());
		Ctrl->SetDetectRange_Hearing(BossAttributeSet->GetDetectRange_Hearing());
	}

	TObjectPtr<UBlackboardComponent> BB = Ctrl->GetBlackboardComponent();
	if (IsValid(BB))
	{
		BB->SetValueAsFloat(TEXT("NAttackDistance"), BossAttributeSet->GetAttackDistance());
		BB->SetValueAsFloat(TEXT("MeleeDistance"), BossAttributeSet->GetBoss_MeleeAttackDistance());
		BB->SetValueAsFloat(TEXT("MiddleDistance"), BossAttributeSet->GetBoss_MiddleAttackDistance());
		BB->SetValueAsFloat(TEXT("RangeDistance"), BossAttributeSet->GetBoss_RangeAttackDistance());
	}

	SetIsBoss(Data->bIsBoss);
}

void AZombieBoss::UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType)
{
	Super::UseAnimMontage(Type, BeforeType);

	//보스 스킬 몽타주가 들어왔는지 확인한다.
	if (Type == EZombieAnimType::Boss_Skill1 ||
		Type == EZombieAnimType::Boss_Skill2 ||
		Type == EZombieAnimType::Boss_Skill3 ||
		Type == EZombieAnimType::Boss_Skill4 ||
		Type == EZombieAnimType::Boss_Skill5 ||
		Type == EZombieAnimType::Boss_Skill6)
	{
		SkillMontageNumber = (int32)(Type);
		AnimInstance->PlaySkillMontage(SkillMontageNumber);
		AnimInstance->SetAnim(EZombieAnimType::CombatIdle);
	}
}

void AZombieBoss::CallBackHP(AActor* InstigatorActor)
{
	Super::CallBackHP(InstigatorActor);
}

void AZombieBoss::CallBackHP(AActor* InstigatorActor, float Damage)
{
	if (!HasAuthority())
	{
		return;
	}

	//기본 사망처리, 공격받았을 때의 타겟 처리는 해줘야 한다.
	CallBackHP(InstigatorActor);

	//공격을 받고 살아남았다면 보스는 추가적으로 받은 데미지를 누적해야 한다.
	if (BossAttributeSet->GetCurrentHP() > 0)
	{
		//FindOrAdd : 있다면 값 참조를 반환, 없다면 추가 후 0을 넣어 값 참조를 반환한다.
		Damageboard.FindOrAdd(InstigatorActor) += Damage;
	}

	float* CurrentDamage = Damageboard.Find(InstigatorActor);
	RS_PRINT_SCREEN(5.f, FColor::Red, TEXT("Player : %s, Damage : %f"), *InstigatorActor->GetName(), *CurrentDamage);
}

void AZombieBoss::InitAbility()
{
	Super::InitAbility();
}

float AZombieBoss::GetHalfHeight()
{
	return 0.f;
}

void AZombieBoss::SetLandingLoc()
{

}

void AZombieBoss::ApplyLandingAttack()
{
}

void AZombieBoss::LaunchProjectile()
{
}

void AZombieBoss::SpawnStone()
{
}

void AZombieBoss::Skill1()
{
}

void AZombieBoss::Skill1End()
{
	if (HasAuthority())
	{
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);
	}
}

void AZombieBoss::Skill2()
{
}

void AZombieBoss::Skill2End()
{
	if (HasAuthority())
	{
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);
	}
}

void AZombieBoss::Skill3()
{
}

void AZombieBoss::Skill3End()
{
	if (HasAuthority())
	{
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);
	}
}

void AZombieBoss::Skill4()
{
}

void AZombieBoss::Skill4End()
{
	if (HasAuthority())
	{
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);
	}
}

void AZombieBoss::Skill5()
{
}

void AZombieBoss::Skill5End()
{
	if (HasAuthority())
	{
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);
	}
}

void AZombieBoss::Skill6()
{
}

void AZombieBoss::Skill6End()
{
	if (HasAuthority())
	{
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);
	}
}

bool AZombieBoss::Skill4Prepared()
{
	return true;
}

void AZombieBoss::ChangeTarget()
{
	if (!HasAuthority())
	{
		return;
	}

	//맵에서 가장 큰 데미지를 준 플레이어를 찾는다.
	APlayerCharacterBase* Target = nullptr;
	float MaxDamage = 0.f;

	for (auto Element : Damageboard)
	{
		APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(Element.Key);
		float TotalDamage = Element.Value;

		//만약 플레이어가 죽은 상태라면 연산에 포함하지 않는다.
		if (Player->IsDead())
		{
			continue;
		}

		if (TotalDamage > MaxDamage)
		{
			Target = Player;
			MaxDamage = TotalDamage;
		}
	}

	//가장 큰 데미지를 준 플레이어를 타겟으로 지정한다.
	AICtrl->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Target);

	//타겟이 설정되었으니 누적 데미지를 초기화한다.
	Damageboard.Reset();
}

bool AZombieBoss::CanUseMeleeAttack_Implementation()
{
	return true;
}

bool AZombieBoss::CanUseMiddleAttack_Implementation()
{
	return true;
}

bool AZombieBoss::CanUseRangeAttack_Implementation()
{
	return true;
}

bool AZombieBoss::CheckOnlyUpperPlay_Implementation()
{
	return false;
}

