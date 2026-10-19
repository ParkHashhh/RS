// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieProjectileBase.h"
#include "Components/BoxComponent.h"
#include "GameFrameWork/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AZombieProjectileBase::AZombieProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 좀비 AI(서버 권한)에서만 스폰되는 액터라서 명시적으로 복제를 켜줘야 클라이언트에도 보임
	bReplicates = true;
	SetReplicateMovement(true);

	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));

	SetRootComponent(Body);
	Mesh->SetupAttachment(Body);
	Niagara->SetupAttachment(Body);

	Movement->SetUpdatedComponent(Body);

	Movement->InitialSpeed = 1000.f;
	Movement->MaxSpeed = 1000.f;
	Movement->bRotationFollowsVelocity = true;

	Movement->OnProjectileStop.AddDynamic(this, &AZombieProjectileBase::ProjectileStop);
}

// Called when the game starts or when spawned
void AZombieProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AZombieProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AZombieProjectileBase::SetOwnerController(AController* Ctrl)
{
	OwnerController = Ctrl;
}

void AZombieProjectileBase::SetAbilityTag(FGameplayTag Tag)
{
	AbilityTag = Tag;
}

void AZombieProjectileBase::ProjectileStop(const FHitResult& ImpactResult)
{
	if (!HasAuthority())
	{
		SpawnGroundHitParticle(ImpactResult);
		return;
	}

	AActor* HitActor = ImpactResult.GetActor();
	if (!HitActor)
	{
		//예외상황에서의 무조건적인 삭제를 보장한다.
		Destroy();
		return;
	}

	APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(HitActor);
	if (Player)
	{
		FGameplayEventData EventData;

		EventData.Target = HitActor;
		EventData.Instigator = OwnerController->GetPawn();
		EventData.EventTag = AbilityTag;

		FGameplayAbilityTargetDataHandle TargetDataHandle;
		FGameplayAbilityTargetData_SingleTargetHit* TargetData =
			new FGameplayAbilityTargetData_SingleTargetHit(ImpactResult);
		TargetDataHandle.Add(TargetData);

		EventData.TargetData = TargetDataHandle;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerController->GetPawn(), EventData.EventTag, EventData);
	}
	else
	{
		SpawnGroundHitParticle(ImpactResult);
	}

	Destroy();
}

void AZombieProjectileBase::SpawnGroundHitParticle(const FHitResult& ImpactResult)
{
	if (Particle)
	{
		FVector SpawnLoc = ImpactResult.ImpactPoint;
		FRotator SpawnRot = GetActorRotation();
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, SpawnLoc, SpawnRot);
	}
	else if (NiagaraParticle)
	{
		FVector SpawnLoc = ImpactResult.ImpactPoint;
		FRotator SpawnRot = GetActorRotation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), NiagaraParticle, SpawnLoc, SpawnRot);
	}
}

