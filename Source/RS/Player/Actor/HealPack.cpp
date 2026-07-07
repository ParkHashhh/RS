// Fill out your copyright notice in the Description page of Project Settings.


#include "HealPack.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"       
#include "GameFramework/ProjectileMovementComponent.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"       
#include "RS/LogCategory.h"

AHealPack::AHealPack()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CollisionComp;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	CollisionComp->SetBoxExtent(FVector(13.f, 13.f, 5.f));
	CollisionComp->SetCollisionProfileName(TEXT("HealPack"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AHealPack::OnHit);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bShouldBounce = true;	
	ProjectileMovement->Bounciness = 0.2f;
	ProjectileMovement->Friction = 0.1f;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	bReplicates = true;
	SetReplicateMovement(true);
}

void AHealPack::BeginPlay()
{
	Super::BeginPlay();
	if (!LaunchDir.IsZero())
	{
		ProjectileMovement->Velocity = LaunchDir * ProjectileMovement->InitialSpeed;
	}
	
}

void AHealPack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHealPack::Explode()
{
	if (IsValid(HealingNiagara))
	{
		NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HealingNiagara, GetActorLocation());
	}
	if (IsValid(Sound))
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
	GetWorldTimerManager().SetTimer(HealingTimerHandle, this, &AHealPack::PerformHealing, 1.0f, true);

}

void AHealPack::PerformHealing()
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> IgnoreActors;

	const float HealingRadius = 300.f;

	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GetActorLocation(), HealingRadius,
		{ UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel3) }, APlayerCharacterBase::StaticClass(),
		IgnoreActors, OverlappingActors);


	NiagaraComponent->ResetSystem();

	for (AActor* Actor : OverlappingActors)
	{

		APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(GetOwner());
		if (IsValid(Player))
		{
			Player->Server_ApplyHP(Actor, FHitResult(), 10.f);
		}
	}
	HealingTicks++;
	if (HealingTicks >= MaxHealingTicks)
	{
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->Deactivate();
			NiagaraComponent->DestroyComponent();
		}
		GetWorldTimerManager().ClearTimer(HealingTimerHandle);
		Destroy();
	}
}


void AHealPack::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	const float CheckFloor = 0.8f;
	if (Hit.ImpactNormal.Z < CheckFloor)
	{	
		return;
	}
	if (bExploded)
		return;
	ProjectileMovement->Deactivate();
	bExploded = true;
	Explode();
}
