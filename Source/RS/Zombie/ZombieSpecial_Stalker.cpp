// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieSpecial_Stalker.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RS/Zombie/Animation/ZombieAnimInstance.h"
#include "RS/Player/PlayerCharacterBase.h"
#include "RS/LogCategory.h"
#include "Net/UnrealNetwork.h"
#include "RS/Zombie/GAS/ZombieStandardAttributeSet.h"


AZombieSpecial_Stalker::AZombieSpecial_Stalker()
{
	//특수 좀비의 바디, 메쉬 결정
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	//바디 메쉬에서는 이제 총알 피격을 담당하지 않는다.
	Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Body->SetCollisionProfileName(ZombieBodyCollisionName);
	Mesh->SetupAttachment(Body);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionProfileName(ZombieMeshCollisionName);

	//일반 좀비보다 살짝 크게
	SetActorScale3D(FVector(1.2, 1.2, 1.2));

	double HalfHeight = 50.0;
	Cast<UCapsuleComponent>(Body)->SetCapsuleHalfHeight(HalfHeight);

	Mesh->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -HalfHeight), FRotator(0.0, -90.0, 0.0));

	//스토커는 도약 전에 플레이어 시야각 밖으로 이동하는 준비가 필요하다.
	bNeedPreparationToSkill = true;
}

void AZombieSpecial_Stalker::BeginPlay()
{
	Super::BeginPlay();

	//기본 투명도를 반투명으로 설정한다.
	bIsOpacity = false;
	SetStalkerOpacity(0.5f);

	//쿨다운 태그 변화시 호출될 함수를 델리게이트로 연결한다.
	CooldownTagHandle = ASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.SZSkillCooldown")),
		EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AZombieSpecial_Stalker::OnCooldownTagChange);
}

void AZombieSpecial_Stalker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//스토킹 진행중 불리언 변수가 true인 경우
	if (bIsInStalking)
	{
		UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
		if (!IsValid(BB))
		{
			return;
		}

		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
		if (!IsValid(Target))
		{
			return;
		}

		FVector TargetLoc = Target->GetActorLocation();
		FVector ZombieLoc = GetActorLocation();
		float CurrentSpeed = Movement->MaxSpeed * 1.5f;

		//좀비가 플레이어의 시야각 밖으로 나갔는지 확인한다.
		FVector TargetForward = Target->GetActorForwardVector();
		FVector TargetToZombie = ZombieLoc - TargetLoc;
		FVector TToZNormal = TargetToZombie.GetSafeNormal2D();

		double DotResult = FVector::DotProduct(TargetForward, TToZNormal);

		//플레이어의 시야각 밖으로 나갔는지 확인한다.
		float degree = 45.f;
		float Radian = FMath::DegreesToRadians(degree);
		float CosResult = FMath::Cos(Radian);

		//cos 값보다 작으면 시야각을 벗어난 것이다.
		if (DotResult < CosResult)
		{
			//시야각을 벗어난 상태에서 점프 경로에 장애물이 없을 때 도약 공격을 수행한다.
			if (AICtrl->LineOfSightTo(Target))
			{
				//스토킹을 종료한다.
				bIsInStalking = false;

				//도약 준비 자세를 위해 무브먼트를 정지시킨다.
				if (Movement)
				{
					Movement->StopMovementImmediately();
					Movement->Deactivate();
				}

				//타겟쪽으로 머리를 돌린다.
				FRotator PlayerRot = Target->GetActorRotation();
				FRotator ZombieRot = GetActorRotation();
				FRotator ToTargetRot = UKismetMathLibrary::FindLookAtRotation(ZombieLoc, TargetLoc);
				SetActorRotation(ToTargetRot);

				//블랙보드에 사전 준비가 완료되었음을 알린다.
				BB->SetValueAsBool(TEXT("IsInPreparation"), false);

				return;
			}
		}

		//이 순간 좀비가 플레이어 정면 벡터를 기준으로 왼쪽인지 오른쪽인지를 구한다.
		//이 값에 따라 각도를 점점 더할지 뺄지를 정해야 하기 때문이다.
		FVector CrossResult = FVector::CrossProduct(TargetForward, TToZNormal);

		//외적 결과 Z값이 양수라면 플레이어를 기준으로 좀비는 왼쪽에 존재한다.
		//좀비를 기준으로 하면 플레이어 정면 벡터 오른쪽에 존재하는 것이다.
		//이 상태에서는 좀비가 반시계로 돌아야 시야각 내부로 더 들어가지 않게 된다.
		//반시계로 돌려면 각도를 +를 해줘야 한다.
		float RotateDir;

		if (CrossResult.Z >= 0.f)
		{
			RotateDir = 1.f;
		}
		else
		{
			RotateDir = -1.f;
		}

		//좀비 이동속도를 이용해 각속도를 구한다.
		float AngleSpeed = CurrentSpeed / RealStalkingDistance;
		float DegreeSpeed = FMath::RadiansToDegrees(AngleSpeed);

		//다음 이동 지점 계산을 위한 각도를 구한다.
		FirstDegree += RotateDir * DeltaTime * DegreeSpeed;

		//각도를 라디안으로 변환한다.
		float NextRadian = FMath::DegreesToRadians(FirstDegree);

		//다음 궤도상 좌표를 만든다.
		FVector NextOrbitLoc;
		NextOrbitLoc.X = TargetLoc.X + (FMath::Cos(NextRadian) * RealStalkingDistance);
		NextOrbitLoc.Y = TargetLoc.Y + (FMath::Sin(NextRadian) * RealStalkingDistance);
		NextOrbitLoc.Z = TargetLoc.Z;

		//구한 좌표로 이동시킨다.
		AICtrl->MoveToLocation(NextOrbitLoc, -1.f, false, true, true, false);
	}

	//도약공격의 이동 상황은 이곳에서 갱신한다.
	if (bIsJumping)
	{
		CurrentTime += DeltaTime;

		//점프 진행 시간을 이용해 점프 진행률을 만든다.
		float ProgressRate = FMath::Clamp(CurrentTime / JumpTime, 0, 1);

		//다음 이동 경로를 점프 진행률을 이용해 만든다.
		FVector NextLoc = FMath::Lerp(ZombieLocBeforeJump, TargetLocBeforeJump, ProgressRate);

		if (JumpCurve)
		{
			//등록한 커브를 통해 진행률에 맞는 높이를 가져온다.
			float CurveValue = JumpCurve->GetFloatValue(ProgressRate);
			NextLoc.Z += JumpMaxHeight * CurveValue;
		}

		//구한 다음 이동 좌표로 액터를 이동시킨다.
		//Sweep을 켜서 충돌을 하도록 한다
		FHitResult Hit;
		SetActorLocation(NextLoc, true, &Hit);

		//안전장치 : 충돌이 일어났다면 바닥으로 강제로 내려준다.
		//IsValidBlockingHit : '이동을 하다가' 충돌이 생긴 경우 true 반환
		if (Hit.IsValidBlockingHit())
		{
			//만약 충돌한 물체가 플레이어라면 어빌리티를 발동시킨다.
			APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(Hit.GetActor());
			if (IsValid(Player))
			{
				FGameplayEventData EventData;

				EventData.Target = Player;
				EventData.Instigator =this;
				EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.SZSkill"));

				FGameplayAbilityTargetDataHandle TargetDataHandle;
				FGameplayAbilityTargetData_SingleTargetHit* TargetData =
					new FGameplayAbilityTargetData_SingleTargetHit(Hit);
				TargetDataHandle.Add(TargetData);

				EventData.TargetData = TargetDataHandle;

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
			}

			//충돌 후 허공에 멈추는 것을 막기 위해 강제 위치 보정을 한다.
			FHitResult LandHit;
			FVector StartLoc = GetActorLocation();
			FVector EndLoc = StartLoc - FVector(0.0, 0.0, 1000.0);

			//지면과만 충돌하도록 파라미터를 설정한다 (공중 충돌시 위치 보정을 위한 라인트레이싱 파라미터)
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			Params.AddIgnoredActor(Hit.GetActor());

			if (GetWorld()->LineTraceSingleByChannel(LandHit, StartLoc, EndLoc, ECC_Visibility, Params))
			{
				//지면 (땅, 물체 등)에 충돌했다면 그 위치로 내려준다.
				FVector LandLoc = LandHit.ImpactPoint;
				float HalfHeight = GetHalfHeight();

				//캡슐 절반 높이만큼 올린다 : 발이 땅에 닿아야 하므로
				LandLoc.Z += HalfHeight;

				//구한 위치로 이동시켜 지면으로 내린다.
				SetActorLocation(LandLoc);

				//착지 애니메이션으로 변경한다.
				if (HasAuthority())
				{
					Multicast_LeapAttackEnd();
				}

				//점프 상태 종료
				bIsJumping = false;
				return;
			}
		}

		//점프가 끊기지 않고 진행률이 100%가 되었다면 몽타주 섹션을 착지 섹션으로 바꾼다.
		if (ProgressRate >= 1.f || CurrentTime >= JumpTime)
		{
			if (HasAuthority())
			{
				Multicast_LeapAttackEnd();
			}

			bIsJumping = false;
		}
	}
}

void AZombieSpecial_Stalker::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZombieSpecial_Stalker, bIsOpacity)
}

void AZombieSpecial_Stalker::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.SZSkillCooldown")),
		EGameplayTagEventType::NewOrRemoved).Remove(CooldownTagHandle);

	Super::EndPlay(EndPlayReason);
}

void AZombieSpecial_Stalker::InitStatData(const FZombieDefaultInfo* Data)
{
	Super::InitStatData(Data);

	//스토커는 스토킹 거리가 추적 중단 거리이므로 이곳에서 따로 설정을 다시 해준다.
	TObjectPtr<AZombieAIController> Ctrl = GetController<AZombieAIController>();
	if (IsValid(Ctrl))
	{
		UBlackboardComponent* BB = Ctrl->GetBlackboardComponent();
		if (IsValid(BB))
		{
			BB->SetValueAsFloat(TEXT("TraceEndDistance"), InitStalkingDistance);
		}
	}
}

void AZombieSpecial_Stalker::InitAbility()
{
	Super::InitAbility();

	if (SkillAbility)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(SkillAbility, 1));
	}
}

void AZombieSpecial_Stalker::Attack()
{
	Super::Attack();
}

void AZombieSpecial_Stalker::SkillStart()
{
	//도약 공격을 수행한다.
	//도약 직전 좀비 위치와 타겟 위치를 저장한다 (점프 진행률 계산)
	UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
	if (!IsValid(BB))
	{
		return;
	}
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
	if (!IsValid(Target))
	{
		return;
	}
	TargetLocBeforeJump = Target->GetActorLocation();
	ZombieLocBeforeJump = GetActorLocation();

	//시간 누적변수를 초기화한다.
	CurrentTime = 0.f;

	//도약을 시작한다.
	bIsJumping = true;

	//공격이 시작될 것이므로 불투명으로 바꾼다.
	bIsOpacity = true;
	if (HasAuthority())
	{
		SetStalkerOpacity(1.f);
	}

	//도약공격 사운드를 재생한다 (기합)
	PlayZombieSkillSound();
}

void AZombieSpecial_Stalker::SkillEnd()
{
	Super::SkillEnd();

	//무브먼트를 다시 활성화 한다.
	if (Movement)
	{
		Movement->Activate();
	}

	//가속시켰던 속도를 원상복구 시킨다.
	ChangeSpeed_Run();
}

float AZombieSpecial_Stalker::GetHalfHeight()
{
	return Cast<UCapsuleComponent>(Body)->GetUnscaledCapsuleHalfHeight();
}

void AZombieSpecial_Stalker::PreparationSkill()
{
	Super::PreparationSkill();

	//플레이어 중심의 원 궤도를 돌며 스토킹 해야하므로 이곳에서 불리언 변수를 true로 바꾼다.
	bIsInStalking = true;

	UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
	if (!IsValid(BB))
	{
		return;
	}

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
	if (!IsValid(Target))
	{
		return;
	}

	//아크탄젠트를 이용해 도달 순간 좀비가 몇 도에 위치하는지를 저장한다.
	FVector PlayerLoc = Target->GetActorLocation();
	FVector ZombieLoc = GetActorLocation();
	FVector PlayerToZombie = ZombieLoc - PlayerLoc;

	float Radian = FMath::Atan2(PlayerToZombie.Y, PlayerToZombie.X);

	float Degree = FMath::RadiansToDegrees(Radian);

	FirstDegree = Degree;

	//지금 이 순간 좀비와 플레이어의 거리가 새로운 스토킹 거리가 된다.
	float Distance = FVector::Dist(PlayerLoc, ZombieLoc);

	if (Distance >= InitStalkingDistance)
	{
		RealStalkingDistance = InitStalkingDistance;
	}
	else
	{
		RealStalkingDistance = Distance;
	}

	//달리기 애니메이션을 재생하도록 한다.
	ChangeAnim(EZombieAnimType::Run);
	ChangeSpeed_Run();

	//다시 스토킹 모드에 진입해야 하므로 반투명으로 조정한다.
	bIsOpacity = false;
	if (HasAuthority())
	{
		SetStalkerOpacity(0.5f);
	}
}

void AZombieSpecial_Stalker::SetStalkerOpacity(float Value)
{
	if (MaterialArray.IsEmpty())
	{
		return;
	}

	for (auto& Mtrl : MaterialArray)
	{
		Mtrl->SetScalarParameterValue(TEXT("Opacity"), Value);
	}
}

void AZombieSpecial_Stalker::OnCooldownTagChange(const FGameplayTag Tag, int32 NewCount)
{
	if (!HasAuthority())
	{
		return;
	}

	UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
	if (!IsValid(BB))
	{
		return;
	}

	if (NewCount > 0)
	{
		//쿨타임 태그가 추가되면 추적 중단 거리는 일반공격 사거리를 적용한다.
		BB->SetValueAsFloat(TEXT("TraceEndDistance"), SpecialAttributeSet->GetAttackDistance());
	}
	else
	{
		//쿨타임 태그가 제거되면 추적 중단 거리는 스킬 사거리를 적용한다.
		BB->SetValueAsFloat(TEXT("TraceEndDistance"), InitStalkingDistance);
	}
}

void AZombieSpecial_Stalker::OnRep_ChangeOpacity()
{
	if (HasAuthority())
	{
		return;
	}

	if (bIsOpacity)
	{
		SetStalkerOpacity(1.f);
	}
	else
	{
		SetStalkerOpacity(0.5f);
	}
}

void AZombieSpecial_Stalker::Multicast_LeapAttackEnd_Implementation()
{
	AnimInstance->StalkerLeapAttackEndSection();
}


