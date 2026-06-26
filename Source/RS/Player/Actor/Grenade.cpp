// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"       
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGrenade::AGrenade()
{
	PrimaryActorTick.bCanEverTick = true;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	RootComponent = GrenadeMesh;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(RootComponent);
	CollisionComp->SetSphereRadius(10.0f);
	CollisionComp->SetCollisionProfileName("Player");
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CollisionComp->SetNotifyRigidBodyCollision(true);
	//CollisionComp->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);
	CollisionComp->SetVisibility(true);

	TracerEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TracerEffect"));
	TracerEffect->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = RootComponent;
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;

	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.3f;

	ProjectileMovement->ProjectileGravityScale = 1.0f;

	bReplicates = true; 
	SetReplicateMovement(true);

}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AGrenade::Explode, 3.0f, false);
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGrenade::Explode()
{
	if (IsValid(Niagara))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Niagara, GetActorLocation());
	}

	// 주변에 Apply Damage 필요

	Destroy();
}

void AGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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
