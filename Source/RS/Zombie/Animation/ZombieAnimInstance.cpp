// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieAnimInstance.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/Zombie/ZombieSpecial.h"
#include "RS/Zombie/ZombieBoss.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RS/LogCategory.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Navigation/PathFollowingComponent.h"

UZombieAnimInstance::UZombieAnimInstance()
{

}

void UZombieAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
}
void UZombieAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}

void UZombieAnimInstance::PlayAttackMontage(int32 Number)
{
	FName Key = *FString::Printf(TEXT("Attack%d"), Number);
	//만든 키로 몽타주를 찾는다.
	TObjectPtr<UAnimMontage>* FindResult = AnimPresetArray[AnimIndex_Backup].AtkMontageMap.Find(Key);
	if (FindResult)
	{
		UAnimMontage* Montage = *FindResult;
		Montage_Play(Montage);
	}
}

void UZombieAnimInstance::PlaySkillMontage(int32 Number)
{
	FName Key = *FString::Printf(TEXT("Skill%d"), Number);
	//만든 키로 몽타주를 찾는다.
	TObjectPtr<UAnimMontage>* FindResult = AnimPresetArray[AnimIndex_Backup].SkillMontageMap.Find(Key);
	if (FindResult)
	{
		UAnimMontage* Montage = *FindResult;
		Montage_Play(Montage);
	}
}

void UZombieAnimInstance::PlayCommonMontage(const FName& Name)
{
	TObjectPtr<UAnimMontage>* FindResult = AnimPresetArray[AnimIndex_Backup].CommonMontageMap.Find(Name);
	if (FindResult)
	{
		UAnimMontage* Montage = *FindResult;
		Montage_Play(Montage);
	}
}

void UZombieAnimInstance::SetIsTurn(bool IsTurn)
{
	bIsTurn = IsTurn;
}

bool UZombieAnimInstance::ChechRoarAnimValid()
{
	TObjectPtr<UAnimSequence>* RoarAnim = AnimPresetArray[AnimIndex_Backup].AnimMap.Find(TEXT("Roar"));

	if (RoarAnim == nullptr)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void UZombieAnimInstance::FinishJump()
{
	//점프 몽타주가 재생중인지 확인한다.
	TObjectPtr<UAnimMontage>* FindResult = AnimPresetArray[AnimIndex_Backup].CommonMontageMap.Find(TEXT("Jump"));
	if (FindResult)
	{
		UAnimMontage* Montage = *FindResult;
		if (Montage_IsPlaying(Montage))
		{
			Montage_JumpToSection(TEXT("End"), Montage);
		}
	}
}

void UZombieAnimInstance::StalkerLeapAttackEndSection()
{
	//스토커의 리프어택 몽타주가 재생중인지 확인한다. (Skill1)
	TObjectPtr<UAnimMontage>* FindResult = AnimPresetArray[AnimIndex_Backup].SkillMontageMap.Find(TEXT("Skill1"));
	if (FindResult)
	{
		UAnimMontage* Montage = *FindResult;
		if (Montage_IsPlaying(Montage))
		{
			Montage_JumpToSection(TEXT("End"), Montage);
		}
	}
}

void UZombieAnimInstance::BehemothLeapAttackEndSection()
{
	//베히모스의 리프어택 몽타주가 재생중인지 확인한다. (Skill5)
	TObjectPtr<UAnimMontage>* FindResult = AnimPresetArray[AnimIndex_Backup].SkillMontageMap.Find(TEXT("Skill5"));
	if (FindResult)
	{
		UAnimMontage* Montage = *FindResult;
		if (Montage_IsPlaying(Montage))
		{
			Montage_JumpToSection(TEXT("End"), Montage);
		}
	}
}

void UZombieAnimInstance::BehemothThrowingStoneSection()
{
	//베히모스의 쓰로잉 스톤 몽타주가 재생중인지 확인한다. (Skill4)
	TObjectPtr<UAnimMontage>* FindResult = AnimPresetArray[AnimIndex_Backup].SkillMontageMap.Find(TEXT("Skill4"));
	if (FindResult)
	{
		UAnimMontage* Montage = *FindResult;
		if (Montage_IsPlaying(Montage))
		{
			Montage_JumpToSection(TEXT("Throw"), Montage);
		}
	}
}

void UZombieAnimInstance::AnimNotify_Attack()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Attack();
	}

	//좀비 공격 사운드 재생
	Zombie->PlayZombieAttackSound();

	//좀비 공격 파티클 생성
	Zombie->SpawnZombieAttackParticle();
}

void UZombieAnimInstance::AnimNotify_AttackEnd()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		//공격이 끝났음을 알리기 위해 블랙보드의 해당 변수 값을 설정한다.
		TObjectPtr<AAIController> Ctrl = Zombie->GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), false);

		//행동 쿨다운을 부여한다
		//다음 행동이 바로 나오는 것을 막아주기 위함
		UAbilitySystemComponent* ASC = Zombie->GetAbilitySystemComponent();
		TSubclassOf<UGameplayEffect> Effect = Zombie->GetEffect(TEXT("AttackCooldown"));
		if (ASC && IsValid(Effect))
		{
			//이미 쿨다운 태그가 있다면 이펙트 적용 과정을 무시한다.
			if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.AttackCooldown"))))
			{
				return;
			}

			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(Zombie);
			ASC->ApplyGameplayEffectToSelf(Effect->GetDefaultObject<UGameplayEffect>(), 1.f, Context);
		}
	}
}

void UZombieAnimInstance::AnimNotify_Death()
{
	//Death 함수에서는 캡슐의 충돌 제거, 메쉬의 충돌 활성화 등의 작업을 해야 한다.
	//이 작업은 자동으로 동기화가 되지 않아서 클라에서도 설정해줘야 한다.
	//따라서 Death함수는 서버/클라 모두 호출해야 한다.
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (IsValid(Zombie))
	{
		Zombie->Death();
	}
}

void UZombieAnimInstance::AnimNotify_RoarStart()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (IsValid(Zombie))
	{
		Zombie->PlayZombieRoarSound();
	}
}

void UZombieAnimInstance::AnimNotify_RoarEnd()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		TObjectPtr<AAIController> Ctrl = Zombie->GetController<AAIController>();
		Ctrl->GetBlackboardComponent()->SetValueAsBool(TEXT("IsRoar"), false);
	}
}

void UZombieAnimInstance::AnimNotify_RFootStep()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (IsValid(Zombie))
	{
		Zombie->PlayFootStepCue(TEXT("foot_r"));
	}
}

void UZombieAnimInstance::AnimNotify_LFootStep()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (IsValid(Zombie))
	{
		Zombie->PlayFootStepCue(TEXT("foot_l"));
	}
}

void UZombieAnimInstance::AnimNotify_SkillStart()
{
	TObjectPtr<AZombieSpecial> Zombie = Cast<AZombieSpecial>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->SkillStart();
	}
}

void UZombieAnimInstance::AnimNotify_SkillEnd()
{
	TObjectPtr<AZombieSpecial> Zombie = Cast<AZombieSpecial>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->SkillEnd();
	}
}

void UZombieAnimInstance::AnimNotify_MoveStart()
{
	TObjectPtr<AZombieBase> Zombie = Cast<AZombieBase>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		AZombieAIController* Ctrl = Zombie->GetController<AZombieAIController>();
		if (IsValid(Ctrl))
		{
			Ctrl->RollBackSpeed();
		}
	}
}

void UZombieAnimInstance::AnimNotify_BSkill1()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill1();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill1End()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill1End();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill2()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill2();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill2End()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill2End();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill3()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill3();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill3End()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill3End();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill4()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill4();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill4End()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill4End();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill5()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill5();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill5End()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill5End();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill6()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill6();
	}
}

void UZombieAnimInstance::AnimNotify_BSkill6End()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->Skill6End();
	}
}

void UZombieAnimInstance::AnimNotify_BRangeAttack()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->RangeAttack();
	}
}

void UZombieAnimInstance::AnimNotify_BRangeAttackEnd()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->RangeAttackEnd();
	}
}

void UZombieAnimInstance::AnimNotify_DecideLoc()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->SetLandingLoc();
	}
}

void UZombieAnimInstance::AnimNotify_LandingAttack()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->ApplyLandingAttack();
	}
}

void UZombieAnimInstance::AnimNotify_Launch()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->LaunchProjectile();
	}
}

void UZombieAnimInstance::AnimNotify_SpawnStone()
{
	TObjectPtr<AZombieBoss> Zombie = Cast<AZombieBoss>(TryGetPawnOwner());
	if (Zombie->HasAuthority())
	{
		Zombie->SpawnStone();
	}
}
