// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieNormal.h"
#include "GAS/ZombieStandardAttributeSet.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RS/LogCategory.h"
#include "RS/Zombie/AI/ZombieAIController.h"

AZombieNormal::AZombieNormal()
{
	AttributeSet = CreateDefaultSubobject<UZombieStandardAttributeSet>(TEXT("Attribute"));
	UZombieStandardAttributeSet* StandardAttri = Cast<UZombieStandardAttributeSet>(AttributeSet);
	NormalAttributeSet = StandardAttri;
	if (IsValid(NormalAttributeSet))
	{
		ASC->AddAttributeSetSubobject<UZombieStandardAttributeSet>(NormalAttributeSet);

		////테스트를 위해 공격 사거리를 이곳에서 임시로 어트리뷰트셋에 넣어준다.
		//NormalAttributeSet->InitAttackPower(30.f);
		//NormalAttributeSet->InitDefensePower(0.f);
		//NormalAttributeSet->InitCurrentHP(150.f);
		//NormalAttributeSet->InitMaxHP(150.f);
		//NormalAttributeSet->InitAttackDistance(200.f);
	}
}

void AZombieNormal::BeginPlay()
{
	Super::BeginPlay();

	if (!ZombiePresetArray.IsEmpty() && HasAuthority())
	{
		//서버에서 앞으로 사용할 메쉬 번호를 추첨한다.
		MeshIndex = FMath::RandRange(0, ZombiePresetArray.Num() - 1);

		SetZombieMesh();

		SetDynamicMI();
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

void AZombieNormal::InitStatData(const FZombieDefaultInfo* Data)
{
	if (!ASC || !NormalAttributeSet)
	{
		RS_LOG_DEBUG(LogRSInit, Error, TEXT("%s: ASC or AttributeSet is nullptr"), *LOG_LINE_INFO);
		return;
	}

	//스탠다드 어트리뷰트셋에 데이터를 넣어준다.
	NormalAttributeSet->SetAttackPower(Data->AttackPower);
	NormalAttributeSet->SetDefensePower(Data->DefensePower);
	NormalAttributeSet->SetAttackDistance(Data->AttackDistance);
	NormalAttributeSet->SetWalkSpeed(Data->WalkSpeed);
	NormalAttributeSet->SetRunSpeed(Data->RunSpeed);
	NormalAttributeSet->SetMaxHP(Data->MaxHP);
	NormalAttributeSet->SetCurrentHP(Data->MaxHP);
	NormalAttributeSet->SetAttackPower(Data->AttackPower);
	NormalAttributeSet->SetDetectRange_Sight(Data->DetectRange_Sight);

	//초기 스탯 지정이 완료되면 이동속도, 감지거리를 넣어준다.
	ChangeSpeed_Walk();

	TObjectPtr<AZombieAIController> Ctrl = GetController<AZombieAIController>();
	if (IsValid(Ctrl))
	{
		Ctrl->SetDetectRange_Sight(NormalAttributeSet->GetDetectRange_Sight());
	}
}

void AZombieNormal::SetDynamicMI()
{
	//바디 메쉬의 머티리얼을 다이나믹 MI로 전환한다.
	int32 MaterialCount = Mesh->GetNumMaterials();
	for (int32 i = 0; i < MaterialCount; ++i)
	{
		UMaterialInstanceDynamic* Mtrl = Mesh->CreateDynamicMaterialInstance(i);
		MaterialArray.Add(Mtrl);
	}

	//모든 옷 메쉬의 머티리얼을 다이나믹 MI로 전환한다.
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

void AZombieNormal::SetZombieMesh()
{
	//바디 스켈레탈 메쉬 지정
	if (ZombiePresetArray.IsValidIndex(MeshIndex) && ZombiePresetArray[MeshIndex].BodyMesh)
	{
		Mesh->SetSkeletalMesh(ZombiePresetArray[MeshIndex].BodyMesh);
		//좀비 메쉬에서 총알 피격을 담당한다 : query only로 지정해 연산 부하를 최대한 줄인다.
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionProfileName(ZombieMeshCollisionName);
	}

	//옷 스켈레탈 메쉬 지정
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

		//옷 메쉬는 콜리전 자체를 꺼버린다 : 연산 부하를 최대한 감소시킨다.
		SMComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		//생성자가 아니므로 AttachToComponent 를 사용한다
		//KeepRelativeTransform : 상대적 값을 유지하며 부모에게 붙는다. (어차피 옷은 부모 중심에 맞추어 만들어진다.)
		SMComp->AttachToComponent(Mesh, FAttachmentTransformRules::KeepRelativeTransform);

		//메쉬가 자체 연산하지 말고 부모를 따라 움직이도록 설정한다.
		SMComp->SetLeaderPoseComponent(Mesh);

		//이후 머티리얼에 접근해 파라미터를 변경하기 쉽게 하기 위해 컴포넌트를 따로 보관한다.
		ClothMeshCompArray.Add(SMComp);
	}
}

void AZombieNormal::OnRep_ChangeMesh()
{
	SetZombieMesh();

	SetDynamicMI();
}
