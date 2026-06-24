// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal.h"
#include "GAS/ZombieStandardAttributeSet.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RS/LogCategory.h"

AZombieNormal::AZombieNormal()
{
	AttributeSet = CreateDefaultSubobject<UZombieStandardAttributeSet>(TEXT("Attribute"));
	UZombieStandardAttributeSet* StandardAttri = Cast<UZombieStandardAttributeSet>(AttributeSet);
	NormalAttributeSet = StandardAttri;
	if (IsValid(NormalAttributeSet))
	{
		ASC->AddAttributeSetSubobject<UZombieStandardAttributeSet>(NormalAttributeSet);

		//테스트를 위해 공격 사거리를 이곳에서 임시로 어트리뷰트셋에 넣어준다.
		NormalAttributeSet->InitAttackPower(30.f);
		NormalAttributeSet->InitDefensePower(0.f);
		NormalAttributeSet->InitCurrentHP(150.f);
		NormalAttributeSet->InitMaxHP(150.f);
		NormalAttributeSet->InitAttackDistance(200.f);
	}
}

void AZombieNormal::BeginPlay()
{
	Super::BeginPlay();

	//구조체 중 바디를 꺼내와 Mesh에 지정한다.
	if (!ZombiePresetArray.IsEmpty() && HasAuthority())
	{
		MeshIndex = FMath::RandRange(0, ZombiePresetArray.Num() - 1);
		if (ZombiePresetArray[MeshIndex].BodyMesh)
		{
			Mesh->SetSkeletalMesh(ZombiePresetArray[MeshIndex].BodyMesh);
		}

		for (auto& ClothMesh : ZombiePresetArray[MeshIndex].ClothMeshArray)
		{
			if (!IsValid(ClothMesh))
			{
				continue;
			}
			//스켈레탈 메쉬 컴포넌트를 생성한 후 옷 메쉬를 Set 한다.
			TObjectPtr<USkeletalMeshComponent> SMComp = NewObject<USkeletalMeshComponent>(this);

			//런타임 중에 컴포넌트를 생성했다면 등록을 해줘야 한다.
			SMComp->RegisterComponent();

			SMComp->SetSkeletalMesh(ClothMesh);

			//생성자가 아니므로 AttachToComponent 를 사용한다
			//KeepRelativeTransform : 상대적 값을 유지하며 부모에게 붙는다. (어차피 옷은 부모 중심에 맞추어 만들어진다.)
			SMComp->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);

			//메쉬가 자체 연산하지 말고 부모를 따라 움직이도록 설정한다.
			SMComp->SetLeaderPoseComponent(Mesh);

			//이후 머티리얼에 접근해 파라미터를 변경하기 쉽게 하기 위해 컴포넌트를 따로 보관한다.
			ClothMeshCompArray.Add(SMComp);
		}
	}

	NormalAttributeSet = Cast<UZombieStandardAttributeSet>(AttributeSet);

	//시작 지점을 미리 보관하고, 다음 이동 지점을 하나 만들어둔다.
	SpawnPointLocation = GetActorLocation();
	CalcNextPointLocation();
}

void AZombieNormal::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZombieNormal, MeshIndex);
}

void AZombieNormal::Attack()
{
	Super::Attack();

	AAIController* Ctrl = GetController<AAIController>();
	if (IsValid(Ctrl))
	{
		AActor* Target = Cast<AActor>(Ctrl->GetBlackboardComponent()->GetValueAsObject(TEXT("AttackTarget")));
		if (IsValid(Target))
		{
			FGameplayEventData EventData;

			EventData.Target = Target;
			EventData.Instigator = this;
			EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.Attack"));

			FHitResult Hit;
			Hit.bBlockingHit = true;
			Hit.Location = Target->GetActorLocation();
			Hit.ImpactPoint = Target->GetActorLocation();
			Hit.TraceStart = GetActorLocation();
			Hit.TraceEnd = Hit.Location;
			Hit.Component = Cast<UPrimitiveComponent>(Target->GetRootComponent());
			Hit.HitObjectHandle = FActorInstanceHandle(Target);

			FGameplayAbilityTargetData_SingleTargetHit* TargetData =
				new FGameplayAbilityTargetData_SingleTargetHit(Hit);

			EventData.TargetData.Add(TargetData);

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
		}
	}
}

void AZombieNormal::OnRep_ChangeMesh()
{
	if (MeshIndex != -1 && ZombiePresetArray.IsValidIndex(MeshIndex))
	{
		//구조체 중 바디를 꺼내와 Mesh에 지정한다.
		if (ZombiePresetArray[MeshIndex].BodyMesh)
		{
			Mesh->SetSkeletalMesh(ZombiePresetArray[MeshIndex].BodyMesh);
		}

		for (auto& ClothMesh : ZombiePresetArray[MeshIndex].ClothMeshArray)
		{
			if (!IsValid(ClothMesh))
			{
				continue;
			}
			//스켈레탈 메쉬 컴포넌트를 생성한 후 옷 메쉬를 Set 한다.
			TObjectPtr<USkeletalMeshComponent> SMComp = NewObject<USkeletalMeshComponent>(this);

			//런타임 중에 컴포넌트를 생성했다면 등록을 해줘야 한다.
			SMComp->RegisterComponent();

			SMComp->SetSkeletalMesh(ClothMesh);

			//생성자가 아니므로 AttachToComponent 를 사용한다
			//KeepRelativeTransform : 상대적 값을 유지하며 부모에게 붙는다. (어차피 옷은 부모 중심에 맞추어 만들어진다.)
			SMComp->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);

			SMComp->SetLeaderPoseComponent(Mesh);

			ClothMeshCompArray.Add(SMComp);
		}
	}

	//클라의 디졸브 처리를 위해 다이나믹 머티리얼을 배열에 넣어둔다.
	//바디 메쉬의 모든 머티리얼을 바꾸고 배열에 넣는다 (프리셋은 바디여도 머티리얼이 여러개 들어감)
	int32 MaterialCount = Mesh->GetNumMaterials();
	for (int32 i = 0; i < MaterialCount; ++i)
	{
		UMaterialInstanceDynamic* Mtrl = Mesh->CreateDynamicMaterialInstance(i);
		MaterialArray.Add(Mtrl);
	}

	//옷별로 머티리얼을 체크해 변환한 후 전부 배열에 넣는다.
	for (auto& ClothMeshComp : ClothMeshCompArray)
	{
		MaterialCount = ClothMeshComp->GetNumMaterials();
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			UMaterialInstanceDynamic* Mtrl = ClothMeshComp->CreateDynamicMaterialInstance(i);
			MaterialArray.Add(Mtrl);
		}
	}
}
