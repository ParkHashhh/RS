// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZombieBase.h"
#include "ZombieNormal.generated.h"

class USkeletalMesh;
class UZombieStandardAttributeSet;

UCLASS()
class RS_API AZombieNormal : public AZombieBase
{
	GENERATED_BODY()

public:
	AZombieNormal();
protected:
	//좀비의 바디, 옷 배열을 담은 구조체를 블루프린트에서 지정한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	TArray<FZombieSkeletalMeshSet> ZombiePresetArray;

	//부모는 관리의 편의성을 위해 Base로 들고 있으므로 사용 편의성을 위해 캐스팅을 미리 수행한다.
	UPROPERTY()
	TObjectPtr<UZombieStandardAttributeSet> NormalAttributeSet;

	UPROPERTY(ReplicatedUsing = OnRep_ChangeMesh)
	int32 MeshIndex = -1;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void Attack();

public:
	UFUNCTION()
	void OnRep_ChangeMesh();
};
