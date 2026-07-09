// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"

#include "InteractableInterface.h"
#include "Net/UnrealNetwork.h"
#include "RS/LogCategory.h"
#include "RS/Player/PlayerCharacterBase.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractableComponent, Interactor);
}

void UInteractableComponent::SetFocus(bool bInFocus)
{
}

bool UInteractableComponent::StartInteraction(APlayerCharacterBase* InInteractor)
{
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	if (IsOccupied() || InInteractor == nullptr)
	{
		return false;
	}

	Interactor = InInteractor;

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

	CompleteInteraction();

	return true;
}

void UInteractableComponent::CancelInteraction()
{
	if (!GetOwner()->HasAuthority() || !IsOccupied())
	{
		return;
	}

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

	auto* Interactable = Cast<IInteractableInterface>(GetOwner());
	if (Interactable && Interactor.IsValid())
	{
		Interactable->OnInteractionCompleted(Interactor.Get());
	}

	Interactor.Reset();
	OnInteractionStateChanged.Broadcast(false);
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
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

void UInteractableComponent::OnRep_Interactor()
{
	OnInteractionStateChanged.Broadcast(Interactor.IsValid());
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
