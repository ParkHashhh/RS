// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal.h"
#include "GAS/ZombieStandardAttributeSet.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

AZombieNormal::AZombieNormal()
{
	AttributeSet = CreateDefaultSubobject<UZombieStandardAttributeSet>(TEXT("Attribute"));
	UZombieStandardAttributeSet* StandardAttri = Cast<UZombieStandardAttributeSet>(AttributeSet);
	if (IsValid(StandardAttri))
	{
		ASC->AddAttributeSetSubobject<UZombieStandardAttributeSet>(StandardAttri);
		//테스트를 위해 공격 사거리를 이곳에서 임시로 어트리뷰트셋에 넣어준다.
		StandardAttri->InitAttackDistance(200.f);
		StandardAttri->InitAttack(30.f);
	}
}

void AZombieNormal::BeginPlay()
{
	Super::BeginPlay();

	if (!MeshArray.IsEmpty())
	{
		int32 Index = FMath::RandRange(0, MeshArray.Num() - 1);

		if (MeshArray[Index])
		{
			Mesh->SetSkeletalMesh(MeshArray[Index]);
		}
	}

	NormalAttributeSet = Cast<UZombieStandardAttributeSet>(AttributeSet);
}

void AZombieNormal::Attack()
{
	Super::Attack();

	AAIController* Ctrl = GetController<AAIController>();
	if (IsValid(Ctrl))
	{
		AActor* Target = Cast<AActor>(Ctrl->GetBlackboardComponent()->GetValueAsObject(TEXT("AttackTarget")));
		if (IsValid(Target))
		{
			FGameplayEventData EventData;

			EventData.Target = Target;
			EventData.Instigator = this;
			EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.Attack"));

			FHitResult Hit;
			Hit.bBlockingHit = true;
			Hit.Location = Target->GetActorLocation();
			Hit.ImpactPoint = Target->GetActorLocation();
			Hit.TraceStart = GetActorLocation();
			Hit.TraceEnd = Hit.Location;
			Hit.Component = Cast<UPrimitiveComponent>(Target->GetRootComponent());
			Hit.HitObjectHandle = FActorInstanceHandle(Target);

			FGameplayAbilityTargetData_SingleTargetHit* TargetData =
				new FGameplayAbilityTargetData_SingleTargetHit(Hit);

			EventData.TargetData.Add(TargetData);

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
		}
	}
}
