// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"

#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"       
#include "GameFramework/ProjectileMovementComponent.h"
#include "RS/Zombie/ZombieBase.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"       
#include "RS/LogCategory.h"


// Sets default values
AGrenade::AGrenade()
{
	PrimaryActorTick.bCanEverTick = true;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = GrenadeMesh;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->SetupAttachment(RootComponent);
	CollisionComp->SetSphereRadius(10.0f);
	CollisionComp->SetNotifyRigidBodyCollision(true);
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
	if (!LaunchDir.IsZero())
	{
		ProjectileMovement->Velocity = LaunchDir * ProjectileMovement->InitialSpeed;
	}

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
	if (IsValid(Sound))
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, GetActorLocation());
	}
	// 주변에 Apply Damage 필요
	const float ExplosionRadius = 300.f;

	TArray<AActor*> OverlappingActors;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);


	UKismetSystemLibrary::SphereOverlapActors(GetWorld(),GetActorLocation(),ExplosionRadius,
		{ UEngineTypes::ConvertToObjectType(ECC_GameTraceChannel2) },AZombieBase::StaticClass(),
		IgnoreActors,OverlappingActors);
	RS_PRINT_SCREEN(5.f, FColor::Yellow, TEXT("Found Actors: %d"), OverlappingActors.Num());
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
		RS_PRINT_SCREEN(10.f, FColor::Red, TEXT("Zombie Actor"));
		APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(GetOwner());
		if (IsValid(Player))
		{
			Player->Server_ApplyDamage(Actor, FHitResult(), 100.f);
		}
	}

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
