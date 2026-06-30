// Fill out your copyright notice in the Description page of Project Settings.


#include "GameplayCueNotify_Zombie_Step.h"
#include "Kismet/GameplayStatics.h"

bool UGameplayCueNotify_Zombie_Step::OnExecute_Implementation(AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	//파라미터를 통해 전달받은 피지컬 머티리얼
	const UPhysicalMaterial* PMtrl = Parameters.PhysicalMaterial.Get();

	if (!IsValid(PMtrl))
	{
		return false;
	}

	EPhysicalSurface Surface = PMtrl->SurfaceType;

	//맵에서 꺼내온 사운드 큐 포인터가 유효한지 확인 : 머티리얼은 있는데 Surface 지정을 안한 경우도 있으므로
	const TObjectPtr<USoundBase>* SoundCuePtr = SoundCueMap.Find(Surface);

	if (!SoundCuePtr)
	{
		return false;
	}

	USoundBase* SoundCue = *SoundCuePtr;

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundCue, Parameters.Location, 0.1f);

	return true;
}
