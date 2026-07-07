// Fill out your copyright notice in the Description page of Project Settings.


#include "HealPack.h"
#include "Components/SphereComponent.h"
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

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(RootComponent);
	CollisionComp->SetSphereRadius(20.0f);
	CollisionComp->SetCollisionProfileName(TEXT("HealPack"));
	CollisionComp->SetVisibility(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &AHealPack::OnHit);

	RootComponent = CollisionComp;

	HealPackMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HealPackMesh"));
	HealPackMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealPackMesh->SetupAttachment(RootComponent);


	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->bShouldBounce = false;
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
		// 1. 반환값을 즉시 캐스팅하여 멤버 변수에 저장 (헤더에 선언 필요)
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

	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("OverlappingActorsNum : %d"), OverlappingActors.Num());


	if (OverlappingActors.Num() >= 1)	
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 300.f, 24, FColor::Red, false, 2.0f, 0, 2.0f);
	}
	else
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), 300.f, 24, FColor::Green, false, 2.0f, 0, 2.0f);
	}

	for (AActor* Actor : OverlappingActors)
	{

		APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(GetOwner());
		if (IsValid(Player))
		{
			Player->Server_ApplyDamage(Actor, FHitResult(), -10.f);
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
	RS_PRINT_SCREEN(1.f, FColor::Green, TEXT("OnHit"));

	if (bExploded)
		return;
	bExploded = true;
	Explode();
}
