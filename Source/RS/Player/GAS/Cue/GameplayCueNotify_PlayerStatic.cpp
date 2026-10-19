// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/Player/GAS/Cue/GameplayCueNotify_PlayerStatic.h"
#include "Kismet/GameplayStatics.h"       
#include "NiagaraFunctionLibrary.h"       
#include "Sound/SoundBase.h"              
#include "Particles/ParticleSystem.h"     
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "RS/Core/LogCategory.h"

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
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, SpawnLocation);
	}

	const FSurfaceEffectData* FoundEffect = nullptr;

	if (Hit && Hit->PhysMaterial.IsValid())
	{
		FoundEffect = AttackEffectMap.Find(Hit->PhysMaterial.Get());
	}

	if (!FoundEffect && Parameters.PhysicalMaterial.IsValid())
	{
		FoundEffect = AttackEffectMap.Find(Parameters.PhysicalMaterial.Get());
	}

	FRotator SplashRotation = SpawnRotation;

	if (FoundEffect)
	{
		// 스플래터 생성
		if (IsValid(FoundEffect->SplatterMaterial))
		{
			UGameplayStatics::SpawnDecalAtLocation(Target->GetWorld(), FoundEffect->SplatterMaterial,
				SurfaceSplashSize, SpawnLocation, SpawnRotation, 4.f);
		}

		// 데칼 생성
		if (IsValid(FoundEffect->DecalMaterial))
		{
			UGameplayStatics::SpawnDecalAtLocation(Target->GetWorld(), FoundEffect->DecalMaterial,
				SurfaceDecalSize, SpawnLocation, SpawnRotation, 4.f);
		}
		// 나이아가라 생성
		if (IsValid(FoundEffect->SplatterNiagara))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target->GetWorld(), FoundEffect->SplatterNiagara, SpawnLocation, SpawnRotation, SurfaceNiagaraSize);
		}

		// 나이아가라 생성
		if (IsValid(FoundEffect->Sound))
		{
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), FoundEffect->Sound, SpawnLocation);
		}
	}
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
