// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCueNotify_StaticBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "RS/Core/LogCategory.h"

UGameplayCueNotify_StaticBase::UGameplayCueNotify_StaticBase()
{
}

bool UGameplayCueNotify_StaticBase::OnExecute_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{

	FVector SpawnLocation = Target->GetActorLocation();
	if (IsValid(Particle))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, SpawnLocation);
	}

	if (IsValid(Niagara))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Niagara, SpawnLocation);

	}

	if (IsValid(Sound))
	{
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, SpawnLocation);
	}

	return true;
}

bool UGameplayCueNotify_StaticBase::OnActive_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	return true;
}

bool UGameplayCueNotify_StaticBase::WhileActive_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	return true;
}

bool UGameplayCueNotify_StaticBase::OnRemove_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	return true;
}
