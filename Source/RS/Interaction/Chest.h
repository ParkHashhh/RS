// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableInterface.h"
#include "GameFramework/Actor.h"
#include "Chest.generated.h"

class APlayerCharacterBase;
class UInteractableComponent;

UCLASS()
class RS_API AChest : public AActor, public IInteractableInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TObjectPtr<UInteractableComponent> Interactable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	float OpenTime = 3.0f;

	// 스테이지 클리어 전까지는 배치된 체스트가 다 숨어있어야 하고, ActiveChest()가 클리어 시점에
	// 인원수만큼만 켬 - BeginPlay에서 이 기본값을 강제 적용하므로 true면 스폰 직후 다 보여버림.
	// 웨어하우스처럼 ActiveChest() 스케일링을 안 타는 맵은 배치한 상자마다 에디터에서 직접 켜야 함
	UPROPERTY(ReplicatedUsing = OnRep_ChestActiveState, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	bool bChestActiveState = false;

public:
	AChest();

	virtual void OnInteractionCompleted(AActor* Interactor) override;

	virtual float GetInteractionDuration() const override;

	const UStaticMeshComponent* GetMesh() const
	{
		return Mesh;
	}

	const UInteractableComponent* GetInteractable() const
	{
		return Interactable;
	}

	float GetOpenTime() const
	{
		return OpenTime;
	}

	void SetActive(bool bActive);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;

private:
	void OnPlayerGetReward(APlayerCharacterBase* Player);
	UFUNCTION()
	void OnRep_ChestActiveState();
};
