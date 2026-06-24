// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCueNotify_PlayerStatic.h"
#include "Kismet/GameplayStatics.h"       
#include "NiagaraFunctionLibrary.h"       
#include "Sound/SoundBase.h"              
#include "Particles/ParticleSystem.h"     
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "RS/LogCategory.h"

UGameplayCueNotify_PlayerStatic::UGameplayCueNotify_PlayerStatic()
{
}

bool UGameplayCueNotify_PlayerStatic::OnExecute_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{


	FVector SpawnLocation = Target->GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	const FHitResult* Hit = Parameters.EffectContext.GetHitResult();
	if (Hit)
	{
		SpawnLocation = Hit->ImpactPoint;
		SpawnRotation = Hit->ImpactNormal.Rotation();
	}
	else
	{
		// Hit 없으면 CueParams.Location 사용
		SpawnLocation = Parameters.Location;
		SpawnRotation = Parameters.Normal.Rotation();
	}

	if (IsValid(Particle))
	{
		UGameplayStatics::SpawnEmitterAtLocation(Target->GetWorld(), Particle, SpawnLocation, SpawnRotation);
	}

	if (IsValid(Niagara))
	{
		FRotator NiagaraRotation = SpawnRotation;

		NiagaraRotation.Yaw += FMath::RandRange(-NiagaraRandomYawRange, NiagaraRandomYawRange);
		NiagaraRotation.Pitch += FMath::RandRange(-NiagaraRandomPitchRange, NiagaraRandomPitchRange);

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target->GetWorld(), Niagara, SpawnLocation, NiagaraRotation + NiagaraRotationOffset);


	}


	if (IsValid(Sound))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target->GetWorld(), Niagara, SpawnLocation, SpawnRotation);
	}

	if (IsValid(Decal))
	{
		UGameplayStatics::SpawnDecalAtLocation(Target->GetWorld(), Decal, FVector(5.f, 5.f, 5.f), SpawnLocation, SpawnRotation);
	}

	if (IsValid(Splash))
	{
		UGameplayStatics::SpawnDecalAtLocation(Target->GetWorld(), Splash, FVector(15.f, 15.f, 15.f), SpawnLocation, SpawnRotation);
	}

	EPhysicalSurface Surface = SurfaceType_Default;
	FName SurfaceName = NAME_None;
	if (Hit && Hit->PhysMaterial.IsValid())
	{
		SurfaceName = Hit->PhysMaterial->GetFName();
	}
	else if (Parameters.PhysicalMaterial.IsValid())
	{
		SurfaceName = Parameters.PhysicalMaterial->GetFName();
	}

	UMaterialInterface* SurfaceDecal = SurfaceDecalMap.FindRef(SurfaceName);
	UMaterialInterface* SurfaceSplash = SurfaceSplashMap.FindRef(SurfaceName);


	FRotator SplashRotation = SpawnRotation;
	if(Hit)
		SplashRotation = (-Hit->ImpactNormal).Rotation();

	if (IsValid(SurfaceSplash))
		UGameplayStatics::SpawnDecalAtLocation(Target->GetWorld(), SurfaceSplash,
			SurfaceSplashSize, SpawnLocation, SplashRotation, 4.f);

	if (IsValid(SurfaceDecal))
		UGameplayStatics::SpawnDecalAtLocation(Target->GetWorld(), SurfaceDecal,
			SurfaceDecalSize, SpawnLocation, SpawnRotation, 4.f);



	return true;
}

bool UGameplayCueNotify_PlayerStatic::OnActive_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	return true;
}

bool UGameplayCueNotify_PlayerStatic::WhileActive_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	return true;
}

bool UGameplayCueNotify_PlayerStatic::OnRemove_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	return true;
}
