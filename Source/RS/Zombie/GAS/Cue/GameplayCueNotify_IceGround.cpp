// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCueNotify_IceGround.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

bool AGameplayCueNotify_IceGround::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	AttachToActor(MyTarget, FAttachmentTransformRules::SnapToTargetIncludingScale);

	FVector TraceStart = MyTarget->GetActorLocation();
	FVector TraceEnd = TraceStart - FVector(0.f, 0.f, 300.f);

	FHitResult FloorHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(MyTarget);

	if (GetWorld()->LineTraceSingleByChannel(FloorHit, TraceStart, TraceEnd, ECC_Visibility, Params))
	{
		SetActorLocation(FloorHit.ImpactPoint);
	}

	if (IsValid(IceGroundVFX))
	{
		if (UNiagaraComponent* VFXComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			IceGroundVFX, GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, false))
		{
			VFXComp->SetRelativeScale3D(IceGroundScale);
		}
	}

	return true;
}
