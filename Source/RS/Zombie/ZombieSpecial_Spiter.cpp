// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpecial_Spiter.h"
#include "Components/CapsuleComponent.h"
#include "ZombieProjectileBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"


AZombieSpecial_Spiter::AZombieSpecial_Spiter()
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

	double HalfHeight = 90.0;
	Cast<UCapsuleComponent>(Body)->SetCapsuleHalfHeight(HalfHeight);

	Mesh->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -HalfHeight), FRotator(0.0, -90.0, 0.0));
}

void AZombieSpecial_Spiter::SpawnSpitProjectile()
{
	FVector LaunchLoc = Mesh->GetSocketLocation(TEXT("Mouth"));
	FRotator LaunchRot = Mesh->GetSocketRotation(TEXT("Mouth"));

	//입이 타겟을 바라보도록 하는 회전값을 구한다.
	AZombieAIController* Ctrl = Cast<AZombieAIController>(GetController());
	UBlackboardComponent* BB = Ctrl->GetBlackboardComponent();
	AActor* Player = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
	FVector PlayerLoc = Player->GetActorLocation();
	PlayerLoc.Z += 40.f;
	FRotator LocToPlayer = UKismetMathLibrary::FindLookAtRotation(LaunchLoc, PlayerLoc);
	//회전값 중 상하 각도만 사용한다.
	LaunchRot.Pitch = LocToPlayer.Pitch;

	FActorSpawnParameters	param;

	param.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	param.Owner = this;
	param.Instigator = this;

	if (SkillProjectile)
	{
		TObjectPtr<AZombieProjectileBase>	Proj = GetWorld()->SpawnActor<AZombieProjectileBase>(SkillProjectile, LaunchLoc, LaunchRot, param);
		Proj->SetOwnerController(GetController());
	}

	SpawnZombieAttackParticle();
}

void AZombieSpecial_Spiter::InitAbility()
{
	Super::InitAbility();

	if (SkillLaunchAbility)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(SkillLaunchAbility, 1));
	}

	if (SkillHitAbility)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(SkillHitAbility, 1));
	}
}

void AZombieSpecial_Spiter::Attack()
{
	FVector LaunchLoc = Mesh->GetSocketLocation(TEXT("Mouth"));
	FRotator LaunchRot = Mesh->GetSocketRotation(TEXT("Mouth"));

	//입이 타겟을 바라보도록 하는 회전값을 구한다.
	AZombieAIController* Ctrl = Cast<AZombieAIController>(GetController());
	UBlackboardComponent* BB = Ctrl->GetBlackboardComponent();
	AActor* Player = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
	FVector PlayerLoc = Player->GetActorLocation();
	PlayerLoc.Z += 40.f;
	FRotator LocToPlayer = UKismetMathLibrary::FindLookAtRotation(LaunchLoc, PlayerLoc);
	//회전값 중 상하 각도만 사용한다.
	LaunchRot.Pitch = LocToPlayer.Pitch;

	FActorSpawnParameters	param;

	param.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (NormalProjectile)
	{
		TObjectPtr<AZombieProjectileBase>	Proj = GetWorld()->SpawnActor<AZombieProjectileBase>(NormalProjectile, LaunchLoc, LaunchRot, param);
		Proj->SetOwnerController(GetController());
	}
}

void AZombieSpecial_Spiter::SkillStart()
{
	//발사 태그를 넣어서 스킬 어빌리티를 한번 발동한다
	//스킬 쿨타임만 적용시키고 종료된다.
	FGameplayEventData EventData;
	EventData.Instigator = this;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.SZSkillLaunch"));
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);

	StartSpit();

	PlayZombieSkillSound();
}

void AZombieSpecial_Spiter::SkillEnd()
{
	Super::SkillEnd();
	EndSpit();
}

void AZombieSpecial_Spiter::StartSpit()
{
	GetWorld()->GetTimerManager().SetTimer(SpitTimer, this, &AZombieSpecial_Spiter::SpawnSpitProjectile, SpitInterval, true);
}

void AZombieSpecial_Spiter::EndSpit()
{
	GetWorld()->GetTimerManager().ClearTimer(SpitTimer);
}

float AZombieSpecial_Spiter::GetHalfHeight()
{
	return Cast<UCapsuleComponent>(Body)->GetUnscaledCapsuleHalfHeight();
}

void AZombieSpecial_Spiter::SpawnZombieAttackParticle()
{
	//스피터는 입에서 머즐 파티클이 스폰되어야 한다.
	TObjectPtr<UParticleSystem>* ParticlePtr = ParticleMap.Find(TEXT("Attack"));
	if (ParticlePtr)
	{
		UParticleSystem* Particle = *ParticlePtr;
		FVector SpawnLoc = Mesh->GetSocketLocation(TEXT("Mouth"));
		FRotator SpawnRot= Mesh->GetSocketRotation(TEXT("Mouth"));
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, SpawnLoc, SpawnRot, FVector(0.3, 0.3, 0.3));
	}

	TObjectPtr<UNiagaraSystem>* NiagaraPtr = NiagaraParticleMap.Find(TEXT("Attack"));
	if (NiagaraPtr)
	{
		UNiagaraSystem* Niagara = *NiagaraPtr;
		FVector SpawnLoc = Mesh->GetSocketLocation(TEXT("Mouth"));
		FRotator SpawnRot = Mesh->GetSocketRotation(TEXT("Mouth"));
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Niagara, SpawnLoc, SpawnRot, FVector(0.3, 0.3, 0.3));
	}
}
