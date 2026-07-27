// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCueNotify_Poison.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

bool AGameplayCueNotify_Poison::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	AttachToActor(MyTarget, FAttachmentTransformRules::SnapToTargetIncludingScale);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			FString::Printf(TEXT("[PoisonCue] MyTarget = %s"), *MyTarget->GetName()));
	}
	if (IsValid(PoisonVFX))
	{
		if (UNiagaraComponent* VFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			PoisonVFX, GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, false))
		{
			VFXComp->SetRelativeScale3D(PoisonScale);
		}
	}

	return true;
}
