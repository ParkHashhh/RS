// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpecial_Bomber.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RS/Zombie/Animation/ZombieAnimInstance.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/LogCategory.h"
#include "Net/UnrealNetwork.h"
#include "RS/Zombie/GAS/ZombieStandardAttributeSet.h"
#include "Net/UnrealNetwork.h"

AZombieSpecial_Bomber::AZombieSpecial_Bomber()
{
	//특수 좀비의 바디, 메쉬 결정
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	//바디 메쉬에서는 이제 총알 피격을 담당하지 않는다.
	Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Body->SetCollisionProfileName(ZombieBodyCollisionName);
	Mesh->SetupAttachment(Body);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionProfileName(ZombieMeshCollisionName);

	//일반 좀비보다 살짝 크게
	SetActorScale3D(FVector(1.2, 1.2, 1.2));

	double HalfHeight = 30.0;
	Cast<UCapsuleComponent>(Body)->SetCapsuleHalfHeight(HalfHeight);

	Mesh->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -HalfHeight), FRotator(0.0, -90.0, 0.0));

	bNeedPreparationToSkill = false;
}

void AZombieSpecial_Bomber::BeginPlay()
{
	Super::BeginPlay();
}

void AZombieSpecial_Bomber::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZombieSpecial_Bomber, bIsPrintClient)
}

void AZombieSpecial_Bomber::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZombieSpecial_Bomber::InitAbility()
{
	Super::InitAbility();

	if (SkillAbility)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(SkillAbility, 1));
	}
}

void AZombieSpecial_Bomber::SkillStart()
{
	//부들부들 떨며 몸이 살짝 커지는 연출 이곳에서 추가
}

void AZombieSpecial_Bomber::SkillEnd()
{
	Super::SkillEnd();

	//어빌리티를 발동한다.
	Explosion();

	//폭발 연출을 화면에 출력한다.
	SpawnEffect();

	//SkillEnd 함수는 서버일 경우에만 노티파이에서 호출되므로 리플리케이트 변수를 한번 바꿔준다.
	bIsPrintClient = !bIsPrintClient;

	//메쉬를 투명하게 해서 감춘다.
	Mesh->SetVisibility(false, true);
}

float AZombieSpecial_Bomber::GetHalfHeight()
{
	return Cast<UCapsuleComponent>(Body)->GetUnscaledCapsuleHalfHeight();
}

void AZombieSpecial_Bomber::OnEndAbility()
{
	if (!HasAuthority())
	{
		return;
	}

	SetLifeSpan(0.3f);
}

void AZombieSpecial_Bomber::Death()
{
	//어빌리티를 발동한다.
	Explosion();

	//폭발 연출을 화면에 출력한다.
	SpawnEffect();

	//메쉬를 투명하게 한다.
	Mesh->SetVisibility(false, true);
}

void AZombieSpecial_Bomber::Explosion()
{
	//자신 주변으로 원형 영역을 충돌 체크한다.
	AAIController* Ctrl = GetController<AAIController>();
	if (IsValid(Ctrl))
	{
		//좀비의 공격 캡슐을 생성할 위치와 각도를 정한다.
		TArray<FHitResult> HitArray;
		FVector Loc = GetActorLocation();
		Loc.Z -= GetHalfHeight();

		//나 자신은 무시하도록 파라미터를 만든다.
		FCollisionQueryParams Param(NAME_None, false, this);

		//캡슐 모양으로 임시 충돌체를 생성해서 범위 충돌을 체크한다.
		bool Collision = GetWorld()->SweepMultiByChannel(HitArray, Loc, Loc, GetActorQuat(),
			ECollisionChannel::ECC_GameTraceChannel6, FCollisionShape::MakeSphere(300.f), Param);

		FColor DebugColor = FColor::Green;

		FGameplayEventData EventData;
		EventData.Instigator = this;
		EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.SZSkill"));

		if (HitArray.Num() > 0)
		{
			for (auto Hit : HitArray)
			{
				APlayerCharacterBase* HitActor = Cast<APlayerCharacterBase>(Hit.GetActor());
				if (!IsValid(HitActor))
				{
					continue;
				}

				DebugColor = FColor::Red;

				FGameplayAbilityTargetData_SingleTargetHit* TargetData =
					new FGameplayAbilityTargetData_SingleTargetHit(Hit);

				EventData.TargetData.Add(TargetData);

			}
		}

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);

#if WITH_EDITOR
		DrawDebugSphere(GetWorld(), Loc, 300.f, 32, DebugColor, false, 3.f);
#endif
	}
}

void AZombieSpecial_Bomber::SpawnEffect()
{
	if (Particle)
	{
		FVector SpawnLoc = GetActorLocation();
		FRotator SpawnRot = GetActorRotation();
		FVector Scale = FVector(2.f, 2.f, 2.f);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, SpawnLoc, SpawnRot, Scale);
	}
	else if (NiagaraParticle)
	{
		FVector SpawnLoc = GetActorLocation();
		FRotator SpawnRot = GetActorRotation();
		FVector Scale = FVector(2.f, 2.f, 2.f);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraParticle, SpawnLoc, SpawnRot, Scale);
	}
}

void AZombieSpecial_Bomber::OnRep_SpawnEffectClient()
{
	SpawnEffect();
}
