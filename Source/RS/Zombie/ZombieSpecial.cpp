// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpecial.h"
#include "GAS/ZombieStandardAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RS/LogCategory.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "RS/Zombie/Animation/ZombieAnimInstance.h"
#include "RS/GameMode/RSGameState.h"

AZombieSpecial::AZombieSpecial()
{
	AttributeSet = CreateDefaultSubobject<UZombieStandardAttributeSet>(TEXT("Attribute"));
	UZombieStandardAttributeSet* StandardAttri = Cast<UZombieStandardAttributeSet>(AttributeSet);
	SpecialAttributeSet = StandardAttri;
	if (IsValid(StandardAttri))
	{
		ASC->AddAttributeSetSubobject<UZombieStandardAttributeSet>(StandardAttri);
	}
}

void AZombieSpecial::BeginPlay()
{
	Super::BeginPlay();

	SpecialAttributeSet = Cast<UZombieStandardAttributeSet>(AttributeSet);

	if (HasAuthority())
	{
		if (IsValid(AnimInstance) && !AnimInstance->IsAnimPresetArrayEmpty())
		{
			//서버에서 앞으로 사용할 애님 프리셋 인덱스를 추첨한다.
			AnimIndex = FMath::RandRange(0, AnimInstance->GetAnimPresetNum() - 1);

			//호스트도 애님 인스턴스에 인덱스 백업을 넣어둔다.
			OnRep_ChangeAnimSet();
		}
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
	ChangeSpeed_Walk();
}

void AZombieSpecial::PossessedBy(AController * NewController)
{
	Super::PossessedBy(NewController);

	//디테일에서 설정한 일반공격 가능 여부를 블랙보드에 전달한다. (초기화)
	AAIController* Ctrl = Cast<AAIController>(NewController);
	if (Ctrl)
	{
		UBlackboardComponent* BB = Ctrl->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool(TEXT("EnableNormalAttack"), bEnableNormalAttack);
		}
	}
}

void AZombieSpecial::Attack()
{
	Super::Attack();
}

void AZombieSpecial::InitStatData(const FZombieDefaultInfo* Data)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ASC || !SpecialAttributeSet)
	{
		RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: ASC or AttributeSet is nullptr"), *LOG_LINE_INFO);
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	ARSGameState* GS = GetWorld()->GetGameState<ARSGameState>();
	if (GS)
	{
		int32 DifficultScale = GS->GetDifficultyStage() - 1;

		RS_PRINT_SCREEN(5.f, FColor::Yellow, TEXT("DiffcultScale : %d"), DifficultScale);

		//로드한 데이터를 난이도에 따라 값을 추가한다.
		float AttackPower = Data->AttackPower + (1 * DifficultScale);
		float DefensePower = Data->DefensePower + (0.5 * DifficultScale);
		float MaxHP = Data->MaxHP + (30.f * DifficultScale);
		float SkillDamage = Data->Special_SkillDamage + (2 * DifficultScale);
		//float SkillCooldown = Data->Special_SkillCooldown - (1.0 * DifficultScale);
		//if (SkillCooldown < 5.f)
		//{
		//	SkillCooldown = 5.f;
		//}
		float SkillDotDamage = Data->Special_SkillDotDamage + (0.2 * DifficultScale);

		//스탠다드 어트리뷰트셋에 데이터를 넣어준다.
		SpecialAttributeSet->SetAttackPower(AttackPower);
		SpecialAttributeSet->SetDefensePower(DefensePower);
		SpecialAttributeSet->SetAttackDistance(Data->AttackDistance);
		SpecialAttributeSet->SetWalkSpeed(Data->WalkSpeed);
		SpecialAttributeSet->SetRunSpeed(Data->RunSpeed);
		SpecialAttributeSet->SetMaxHP(MaxHP);
		SpecialAttributeSet->SetCurrentHP(MaxHP);
		SpecialAttributeSet->SetDetectRange_Sight(Data->DetectRange_Sight);
		SpecialAttributeSet->SetDetectRange_Hearing(Data->DetectRange_Hearing);
		SpecialAttributeSet->SetSpecial_SkillCooldown(Data->Special_SkillCooldown);
		//SpecialAttributeSet->SetSpecial_SkillCooldown(SkillCooldown);
		SpecialAttributeSet->SetSpecial_SkillDamage(SkillDamage);
		SpecialAttributeSet->SetSpecial_SkillDotDamage(SkillDotDamage);

		TObjectPtr<AZombieAIController> Ctrl = GetController<AZombieAIController>();
		if (IsValid(Ctrl))
		{
			Ctrl->SetDetectRange_Sight(SpecialAttributeSet->GetDetectRange_Sight());
			Ctrl->SetDetectRange_Hearing(SpecialAttributeSet->GetDetectRange_Hearing());
		}

		SetIsBoss(Data->bIsBoss);

		//특수 좀비 또한 공격 사거리가 추적 중단 거리이다.
		UBlackboardComponent* BB = IsValid(Ctrl) ? Ctrl->GetBlackboardComponent() : nullptr;
		if (IsValid(BB))
		{
			BB->SetValueAsFloat(TEXT("TraceEndDistance"), SpecialAttributeSet->GetAttackDistance());

			//스킬 사용 전 사전 준비가 필요한 좀비임을 블랙보드에 등록한다.
			BB->SetValueAsBool(TEXT("IsNeedPreparation"), bNeedPreparationToSkill);
		}
	}
	else
	{
		RS_PRINT_SCREEN(5.f, FColor::Red, TEXT("Before GameState Ready"));
		GetWorld()->GetTimerManager().SetTimer(InitDataTimerHandle, [this, Data]()
			{
				InitStatData(Data);
			}
		, 0.1f, false);
	}
}

float AZombieSpecial::GetHalfHeight()
{
	return 0.f;
}

void AZombieSpecial::SpawnZombieAttackParticle()
{
	Super::SpawnZombieAttackParticle();
}

void AZombieSpecial::PreparationSkill()
{
}

void AZombieSpecial::OnEndAbility()
{
}

void AZombieSpecial::UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType)
{
	Super::UseAnimMontage(Type, BeforeType);

	//특수 스킬 신호가 들어왔다면 특수스킬 몽타주를 재생한다.
	if (Type == EZombieAnimType::Special_Skill)
	{
		if (HasAuthority())
		{
			SkillMontageNumber = FMath::RandRange(1, AnimInstance->GetSkillMontageNum());
		}
		AnimInstance->PlaySkillMontage(SkillMontageNumber);
		//좀비가 공격을 한다면 기저에 재생되고 있는 애니메이션을 전투 대기로 바꿔준다.
		AnimInstance->SetAnim(EZombieAnimType::CombatIdle);
	}
}

void AZombieSpecial::InitAbility()
{
	Super::InitAbility();
}

void AZombieSpecial::SkillStart()
{

}

void AZombieSpecial::SkillEnd()
{
	//블랙보드의 스킬 사용중 변수를 비활성화 한다
	AAIController* Ctrl = GetController<AAIController>();
	if (!IsValid(Ctrl))
	{
		return;
	}
	UBlackboardComponent* BB = Ctrl->GetBlackboardComponent();
	BB->SetValueAsBool(TEXT("UsingSkill"), false);
}
