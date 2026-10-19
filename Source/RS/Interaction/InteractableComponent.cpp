// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"

#include "InteractableInterface.h"
#include "Net/UnrealNetwork.h"
#include "RS/Core/LogCategory.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Player/Controller/PlayerControllerBase.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractableComponent, Interactor);
	DOREPLIFETIME(UInteractableComponent, Holder);
	DOREPLIFETIME(UInteractableComponent, Duration);
	DOREPLIFETIME(UInteractableComponent, ElapsedTime);
}

void UInteractableComponent::SetFocus(bool bInFocus)
{
	if (bIsFocused == bInFocus)
	{
		return;
	}

	bIsFocused = bInFocus;

	// TODO: 처리 필요
}

bool UInteractableComponent::StartInteraction(APlayerCharacterBase* InInteractor)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	// 다른 사람이 선점
	if (IsOccupied() || !InInteractor)
	{
		return false;
	}

	Interactor = InInteractor;

	RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: StartInteraction, Owner: %s, Interactor: %s"),
		*LOG_LINE_INFO, *GetNameSafe(GetOwner()), *GetNameSafe(InInteractor));

	OnInteractionStateChanged.Broadcast(true);

	if (auto* Interactable = Cast<IInteractableInterface>(GetOwner()))
	{
		Duration = Interactable->GetInteractionDuration();
		if (Duration > 0.0f)
		{
			ElapsedTime = 0.0f;

			GetWorld()->GetTimerManager().SetTimer(InteractionTimerHandle, this, &ThisClass::TickInteractionProgress,
			                                       TickInterval, true);

			return true;
		}
	}

	// 즉시 완료
	CompleteInteraction();

	return true;
}

void UInteractableComponent::CancelInteraction()
{
	if (!GetOwner()->HasAuthority() || !IsOccupied())
	{
		return;
	}

	RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: CancelInteraction"), *LOG_LINE_INFO);

	GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);
	Interactor.Reset();
	OnInteractionStateChanged.Broadcast(false);
}

void UInteractableComponent::CompleteInteraction()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(InteractionTimerHandle);

	RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: CompleteInteraction, Owner: %s, Interactor: %s"),
		*LOG_LINE_INFO, *GetNameSafe(GetOwner()), *GetNameSafe(Interactor.Get()));

	auto* Interactable = Cast<IInteractableInterface>(GetOwner());
	if (Interactable && Interactor.IsValid())
	{
		Interactable->OnInteractionCompleted(Interactor.Get());
	}

	Interactor.Reset();
	OnInteractionStateChanged.Broadcast(false);
}

void UInteractableComponent::RemoveHolder()
{
	Holder.Reset();
}

bool UInteractableComponent::IsOccupiedBy(const APlayerCharacterBase* InInteractor) const
{
	return (Interactor.IsValid() && Interactor.Get() == InInteractor) || (Holder.IsValid() && Holder.Get() == InInteractor);
}

bool UInteractableComponent::CanInteract(APlayerCharacterBase* InInteractor) const
{
	if (auto* InteractableInterface = Cast<IInteractableInterface>(GetOwner()))
	{
		return InteractableInterface->CanInteract(InInteractor);
	}

	return false;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEndOverlap);
}

void UInteractableComponent::EndPlay(EEndPlayReason::Type Reason)
{
	RS_LOG_DEBUG(LogRSInteraction, Log, TEXT("%s: End play"), *LOG_LINE_INFO);
	if (const APlayerCharacterBase* Player = Interactor.Get())
	{
		Player->OnHeldInteractableChanged.Broadcast(false);
	}

	Super::EndPlay(Reason);
}

void UInteractableComponent::OnRegister()
{
	Super::OnRegister();

#if WITH_EDITOR

	if (const AActor* Owner = GetOwner())
	{
		if (const auto* Primitive = Owner->FindComponentByClass<UPrimitiveComponent>();
			!Primitive)
		{
			RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: %s 액터에 충돌체(PrimitiveComponent)가 없음"), *LOG_LINE_INFO,
			             *Owner->GetName());
		}
	}

#endif
}

void UInteractableComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                            const FHitResult& SweepResult)
{
	if (auto* Player = Cast<APlayerCharacterBase>(OtherActor))
	{
		Player->RegisterOverlappedInteractable(this);
	}
}

void UInteractableComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto* Player = Cast<APlayerCharacterBase>(OtherActor))
	{
		Player->UnregisterOverlappedInteractable(this);
	}

	if (OtherActor == Interactor)
	{
		CancelInteraction();
	}

	SetFocus(false);
}

void UInteractableComponent::OnRep_Interactor()
{
	OnInteractionStateChanged.Broadcast(Interactor.IsValid());
}

void UInteractableComponent::OnRep_Holder()
{
}

void UInteractableComponent::OnRep_ElapsedTime()
{
	OnInteractionProgressChanged.Broadcast(ElapsedTime, Duration);
}

void UInteractableComponent::TickInteractionProgress()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	ElapsedTime += TickInterval;

	OnInteractionProgressChanged.Broadcast(ElapsedTime, Duration);

	if (ElapsedTime >= Duration)
	{
		CompleteInteraction();
	}
}
