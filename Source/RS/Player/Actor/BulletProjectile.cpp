// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletProjectile.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ABulletProjectile::ABulletProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->SetCollisionProfileName("Player");
	// 플레이어 자신 무시
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionComp->SetNotifyRigidBodyCollision(true);
	CollisionComp->OnComponentHit.AddDynamic(this, &ABulletProjectile::OnHit);
	CollisionComp->InitSphereRadius(5.0f);
	RootComponent = CollisionComp;

	TracerEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TracerEffect"));
	TracerEffect->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 10000.f;
	ProjectileMovement->MaxSpeed = 10000.f;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
}

// Called when the game starts or when spawned
void ABulletProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABulletProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABulletProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	if (TracerEffect)
	{
		TracerEffect->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		TracerEffect->Deactivate();
		TracerEffect->SetAutoDestroy(true);
	}
	Destroy();
}
