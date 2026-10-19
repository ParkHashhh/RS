// Fill out your copyright notice in the Description page of Project Settings.


#include "ZombieBoss_Behemoth.h"
#include "Components/CapsuleComponent.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "RS/Core/LogCategory.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Zombie/ZombieProjectileBase.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "RS/Zombie/AI/ZombieAIController.h"
#include "RS/Zombie/Animation/ZombieAnimInstance.h"
#include "RS/Zombie/Boss/ZombieBoss_Behemoth_PickupPoint.h"
#include "Navigation/PathFollowingComponent.h"
#include "RS/Zombie/GAS/ZombieBossAttributeSet.h"
#include "GameFrameWork/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "RS/Zombie/ZombieSpawnPoint.h"

AZombieBoss_Behemoth::AZombieBoss_Behemoth()
{
	//특수 좀비의 바디, 메쉬 결정
	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	//바디 메쉬에서는 이제 총알 피격을 담당하지 않는다.
	Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Body->SetCollisionProfileName(ZombieBodyCollisionName);
	Mesh->SetupAttachment(Body);

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionProfileName(ZombieMeshCollisionName);

	Mesh->SetRelativeScale3D(FVector(2.5, 2.0, 2.0));

	double HalfHeight = 200.0;
	double Radius = 80.f;
	Cast<UCapsuleComponent>(Body)->SetCapsuleHalfHeight(HalfHeight);
	Cast<UCapsuleComponent>(Body)->SetCapsuleRadius(Radius);

	Mesh->SetRelativeLocationAndRotation(FVector(0.0, 0.0, -HalfHeight), FRotator(0.0, -90.0, 0.0));
}
void AZombieBoss_Behemoth::BeginPlay()
{
	Super::BeginPlay();

	//서버인 경우, 좌표 지정용 액터로부터 좌표를 복사해 가져온다.
	if (HasAuthority())
	{
		if (IsValid(PickupPointActor))
		{
			PickupPointActor->CopyPickupPointArray(PickupPointArray);
		}

	}
}
void AZombieBoss_Behemoth::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}
void AZombieBoss_Behemoth::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//도약공격의 이동 상황은 이곳에서 갱신한다.
	if (bIsJumping)
	{
		CurrentTime += DeltaTime;

		//점프 진행 시간을 이용해 점프 진행률을 만든다.
		float ProgressRate = FMath::Clamp(CurrentTime / JumpTime, 0, 1);

		//다음 이동 경로를 점프 진행률을 이용해 만든다.
		FVector NextLoc = FMath::Lerp(MyLoc, TargetLoc, ProgressRate);

		if (JumpCurve)
		{
			//커브를 등록했다면 점프 진행률에 맞는 커브 데이터를 가져온다.
			float CurveValue = JumpCurve->GetFloatValue(ProgressRate);
			NextLoc.Z += JumpMaxHeight * CurveValue;
		}

		//구한 다음 이동 좌표로 액터를 이동시킨다.
		//Sweep을 켜서 충돌을 하도록 한다 : 벽에 막혀 정지한다.
		FHitResult Hit;
		SetActorLocation(NextLoc, true, &Hit);

		//안전장치 : 충돌이 일어났다면 바닥으로 강제로 내려준다.
		//IsValidBlockingHit : '이동을 하다가' 충돌이 생긴 경우 true 반환
		//점프가 종료될 수 있는 모든 상황에서 라인트레이싱을 이용해 Z축 보정을 한다.
		if (Hit.IsValidBlockingHit() || ProgressRate >= 1.f || CurrentTime >= JumpTime)
		{
			FHitResult LandHit;
			FVector StartLoc = GetActorLocation();
			FVector EndLoc = StartLoc - FVector(0.0, 0.0, 1000.0);

			//나와 플레이어는 충돌을 무시한다.
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(this);
			//점프 진행률이 100%라면 점프 중 충돌 액터가 없을 수 있다.
			if (Hit.GetActor())
			{
				APlayerCharacterBase* CastedActor = Cast<APlayerCharacterBase>(Hit.GetActor());
				if (CastedActor)
				{
					Params.AddIgnoredActor(Hit.GetActor());
				}
			}

			if (GetWorld()->LineTraceSingleByChannel(LandHit, StartLoc, EndLoc, ECC_Visibility, Params))
			{
				//지면 (땅, 물체 등)에 충돌했다면 그 위치로 보스를 내려준다.
				FVector LandLoc = LandHit.ImpactPoint;
				float HalfHeight = GetHalfHeight();

				//캡슐 절반 높이만큼 올린다 : 발이 땅에 닿아야 하므로
				LandLoc.Z += HalfHeight;

				//구한 위치로 보스를 이동시켜 지면으로 내린다.
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
		//if (ProgressRate >= 1.f || CurrentTime >= JumpTime)
		//{
		//	if (HasAuthority())
		//	{
		//		Multicast_LeapAttackEnd();
		//	}

		//	bIsJumping = false;
		//}
	}

	if (bIsPickupStone)
	{
		UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
		float ThrowingRnage = BossAttributeSet->GetBoss_RangeAttackDistance();

		if (!bIsMoveStartPickupStone)
		{
			//보스가 돌을 들고 있는 상태라면 사거리까지 타겟을 향해 이동해야 한다.
			if (IsValid(BB))
			{
				if (IsValid(Target))
				{
					//이동명령은 한번만 수행하도록 한다.
					bIsMoveStartPickupStone = true;
					AICtrl->MoveToActor(Target, -1.f, false, true, false);
				}
			}
		}

		//플레이어와 보스 사이의 거리를 구한다.
		FVector PlayerLoc = Target->GetActorLocation();
		FVector BossLoc = GetActorLocation();
		float Dist = FVector::Dist(PlayerLoc, BossLoc);
		if (Dist < ThrowingRnage)
		{
			AICtrl->StopMovement();
		}

		EPathFollowingStatus::Type MoveResult = AICtrl->GetMoveStatus();
		if (MoveResult == EPathFollowingStatus::Idle)
		{
			if (IsValid(BB))
			{
				if (IsValid(Target))
				{
					//돌을 던지기 전에 플레이어를 향해 머리를 돌린다.
					if (!bIsFinishRotate)
					{
						//플레이어를 바라보는 Rot
						FRotator ToTarget = (Target->GetActorLocation() - GetActorLocation()).Rotation();
						//보스의 Rot
						FRotator CurrentRot = GetActorRotation();
						//부드럽게 회전하기 위한 보간 Rot
						FRotator InterpRot = FMath::RInterpTo(CurrentRot, ToTarget, DeltaTime, 8.f);
						InterpRot.Pitch = CurrentRot.Pitch;
						InterpRot.Roll = CurrentRot.Roll;
						SetActorRotation(InterpRot);

						//임시 로테이터를 만들고 비교한다.
						FRotator TempToTarget = ToTarget;
						FRotator TempCurrent = CurrentRot;
						TempToTarget.Pitch = 0.0;
						TempToTarget.Roll = 0.0;
						TempCurrent.Pitch = 0.0;
						TempCurrent.Roll = 0.0;
						if (TempCurrent.Equals(TempToTarget, 5.0))
						{
							bIsFinishRotate = true;
						}
					}
					else
					{
						//이동에 성공하면 몽타주를 투척 섹션으로 바꿔준다.
						if(HasAuthority())
						{
							Multicast_StartThrowing();
						}

						//돌을 들고 있지 않은 상태로 변경한다.
						bIsPickupStone = false;

						//던지는 동안에는 이동을 하지 못하도록 다시 막아준다.
						if (Movement)
						{
							Movement->StopMovementImmediately();
							Movement->Deactivate();
						}

						//돌을 든 상태에서 이동 변수를 초기화해준다.
						bIsMoveStartPickupStone = false;
					}
				}
			}
		}
	}

	//돌을 들어올리고 있는 중이라면 양손의 위치를 계산해 돌을 같이 이동시켜줘야 한다.
	if (bIsRaiseStone && IsValid(SpawnedStone))
	{
		FVector RHand = Mesh->GetBoneLocation(TEXT("RightHandMiddle1"));
		FVector LHand = Mesh->GetBoneLocation(TEXT("LeftHandMiddle1"));
		FVector CenterLoc = (RHand + LHand) * 0.5f;

		SpawnedStone->SetActorRotation(GetActorRotation());
		SpawnedStone->SetActorLocation(CenterLoc);
	}
}
void AZombieBoss_Behemoth::InitAbility()
{
	Super::InitAbility();

	//베히모스 어빌리티 등록
	for (auto BZSkillAbility : BZSkillAbilityArray)
	{
		if (BZSkillAbility)
		{
			ASC->GiveAbility(FGameplayAbilitySpec(BZSkillAbility, 1));
		}
	}
}
float AZombieBoss_Behemoth::GetHalfHeight()
{
	return Cast<UCapsuleComponent>(Body)->GetUnscaledCapsuleHalfHeight();
}
void AZombieBoss_Behemoth::UseAnimMontage(EZombieAnimType Type, EZombieAnimType BeforeType)
{
	Super::UseAnimMontage(Type, BeforeType);

	//베히모스의 경우에는 스킬4는 몽타주 변경 후 기본 자세를 Run으로 해야 한다.
	if (Type == EZombieAnimType::Boss_Skill4)
	{
		SkillMontageNumber = (int32)(Type);
		AnimInstance->PlaySkillMontage(SkillMontageNumber);
		AnimInstance->SetAnim(EZombieAnimType::Run);
	}
}

void AZombieBoss_Behemoth::Attack()
{
	//베히모스는 공격 시 피격 판정 범위가 더 커야 한다.
	AAIController* Ctrl = GetController<AAIController>();
	if (IsValid(Ctrl))
	{
		//좀비의 공격 캡슐을 생성할 위치와 각도를 정한다.
		TArray<FHitResult> HitArray;
		FVector StartLoc = GetActorLocation() + GetActorForwardVector() * 100.f;
		StartLoc -= GetActorUpVector() * 50.f;
		FVector EndLoc = StartLoc + GetActorForwardVector() * 100.f;
		FQuat CapsuleRot = FQuat::FindBetweenNormals(FVector::UpVector,
			GetActorRightVector());

		//나 자신은 무시하도록 파라미터를 만든다.
		FCollisionQueryParams Param(NAME_None, false, this);

		//캡슐 모양으로 임시 충돌체를 생성해서 범위 충돌을 체크한다.
		bool Collision = GetWorld()->SweepMultiByChannel(HitArray, StartLoc, EndLoc, CapsuleRot, ECollisionChannel::ECC_GameTraceChannel6,
			FCollisionShape::MakeCapsule(100.f, 60.f), Param);

		FColor DebugColor = FColor::Green;

		//SweepMultiByChannel 함수는 블록 충돌판정이 나와야 true를 반환한다.
		//클리빙 어택 허용 : 현재 코드 유지
		//클리빙 어택 금지 : 트레이스와 플레이어가 block이 되도록 하고, 조건을 Collision이 true 인지 체크한다.
		if (HitArray.Num() > 0)
		{
			for (auto Hit : HitArray)
			{
				AActor* HitActor = Hit.GetActor();
				if (!HitActor)
				{
					continue;
				}

				DebugColor = FColor::Red;

				FGameplayEventData EventData;

				EventData.Target = HitActor;
				EventData.Instigator = this;
				EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.Attack"));

				FGameplayAbilityTargetData_SingleTargetHit* TargetData =
					new FGameplayAbilityTargetData_SingleTargetHit(Hit);

				EventData.TargetData.Add(TargetData);

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
			}
		}

#if WITH_EDITOR

		DrawDebugCapsule(GetWorld(), (StartLoc + EndLoc) * 0.5f, 100.f, 60.f, CapsuleRot,
			DebugColor, false, 1.f);

#endif

	}
}
void AZombieBoss_Behemoth::RangeAttack()
{
	//보스만 근거리,원거리 공격을 전부 가지므로 원거리 공격은 이곳에서 따로 처리한다.
}
void AZombieBoss_Behemoth::RangeAttackEnd()
{
}
void AZombieBoss_Behemoth::Skill1()
{
}

void AZombieBoss_Behemoth::Skill1End()
{
	Super::Skill1End();
}

void AZombieBoss_Behemoth::Skill2()
{
	//풋 스톰프 : 발을 내리찍으며 자신 주변에 피해를 준다.
	//자신 주변으로 원형 영역을 충돌 체크한다.
	AAIController* Ctrl = GetController<AAIController>();
	if (IsValid(Ctrl))
	{
		//좀비의 공격 캡슐을 생성할 위치와 각도를 정한다.
		TArray<FHitResult> HitArray;
		FVector Loc = GetActorLocation();
		Loc.Z -= GetHalfHeight();

		//나 자신은 무시하도록 파라미터를 만든다.
		FCollisionQueryParams Param(NAME_None, false, this);

		//캡슐 모양으로 임시 충돌체를 생성해서 범위 충돌을 체크한다.
		bool Collision = GetWorld()->SweepMultiByChannel(HitArray, Loc, Loc, GetActorQuat(),
			ECollisionChannel::ECC_GameTraceChannel6, FCollisionShape::MakeSphere(500.f), Param);

		FColor DebugColor = FColor::Green;

		FGameplayEventData EventData;
		EventData.Instigator = this;
		EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill2"));

		if (HitArray.Num() > 0)
		{
			for (auto Hit : HitArray)
			{
				APlayerCharacterBase* HitActor = Cast<APlayerCharacterBase>(Hit.GetActor());
				if (!IsValid(HitActor))
				{
					continue;
				}

				DebugColor = FColor::Red;

				FGameplayAbilityTargetData_SingleTargetHit* TargetData =
					new FGameplayAbilityTargetData_SingleTargetHit(Hit);

				EventData.TargetData.Add(TargetData);

			}
		}

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);

#if WITH_EDITOR
		DrawDebugSphere(GetWorld(), Loc, 500.f, 32, DebugColor, false, 3.f);
#endif

	}

	if (HasAuthority())
	{
		//풋 스톰프 : 2번 스킬
		PlayBossSkillSound(2);
		PlayBossSkillEffect(2);
	}

}

void AZombieBoss_Behemoth::Skill2End()
{
	Super::Skill2End();
}

void AZombieBoss_Behemoth::Skill3()
{
	//그라운드 슬램 : 전방 부채꼴로 땅을 타고 날아가는 투사체를 발사한다.
	// 보스 전방 바닥에서 발사
	FVector LaunchLoc = GetActorLocation();
	LaunchLoc += GetActorForwardVector() * 100.f;
	LaunchLoc.Z -= (GetHalfHeight() -30.f);

	FRotator Rot = GetActorRotation();

	FActorSpawnParameters	param;

	param.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	param.Owner = this;
	param.Instigator = this;

	//갯수만큼 부채꼴로 펼쳐서 발사한다.
	float StartYaw = Rot.Yaw - (GSDegree / 2.f);
	float DegreeInterval = GSDegree / (float)GSProjectileNumber;

	if (GroundSlamProjectile)
	{
		for (int32 i = 0; i < GSProjectileNumber; ++i)
		{
			FRotator LaunchRot = FRotator(0.f, StartYaw + (DegreeInterval * i), 0.f);

			TObjectPtr<AZombieProjectileBase>	Proj = GetWorld()->SpawnActor<AZombieProjectileBase>(GroundSlamProjectile, LaunchLoc, LaunchRot, param);
			Proj->SetOwnerController(GetController());

		}
	}

	//스킬을 사용한 시점에서 쿨타임을 적용한다.
	FGameplayEventData EventData;
	EventData.Target = this;
	EventData.Instigator = this;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill3EarlyCooldown"));
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);

	if (HasAuthority())
	{
		//그라운드 슬램 : 3번 스킬
		PlayBossSkillSound(3);
		PlayBossSkillEffect(3);
	}
}

void AZombieBoss_Behemoth::Skill3End()
{
	Super::Skill3End();
}

void AZombieBoss_Behemoth::Skill4()
{
	//쓰로잉 스톤 : 돌을 투척해 피해를 준다.
	//픽업 장소에 도착해 돌을 머리위로 들어올린 시점에 호출된다.
	//무브먼트를 복구한다.
	if (Movement)
	{
		Movement->Activate();
	}

	//돌을 들고 있는 상태로 바꿔준다.
	bIsPickupStone = true;

	//돌을 들고 이동을 하면 하체는 몽타주를 재생하면 안된다.
	bIsOnlyUpperPlay = true;
}

void AZombieBoss_Behemoth::Skill4End()
{
	Super::Skill4End();

	//마지막 투척 섹션 재생을 위해 막아둔 무브먼트를 복구한다.
	if (Movement)
	{
		Movement->Activate();
	}

	//사용한 변수를 초기화한다 : 돌던지기 전 머리를 돌렸는지 확인하는 변수
	bIsFinishRotate = false;
}

void AZombieBoss_Behemoth::Skill5()
{
	//리프 어택 : 타겟을 향해 도약 후 착지하며 주변에 피해를 준다.
	//플로팅 폰 무브먼트를 정지시킨다.
	if (Movement)
	{
		Movement->StopMovementImmediately();
		Movement->Deactivate();
	}

	//시간 누적변수를 초기화한다.
	CurrentTime = 0.f;
}

void AZombieBoss_Behemoth::Skill5End()
{
	Super::Skill5End();
	//착지 모션이 종료되는 타이밍에 호출된다.
	//무브먼트를 다시 활성화 한다.
	if (Movement)
	{
		Movement->Activate();
	}
}

void AZombieBoss_Behemoth::Skill6()
{
	//좀비 소환 : 포효를 지르며 일반 좀비를 소환해 타겟에게 돌진시킨다.
	//소환 자체는 어빌리티로 하는 것이 아니므로 쿨타임 태그가 있는지 확인한다.
	if (!ASC || ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill6Cooldown"))))
	{
		return;
	}

	//보스가 가진 스폰 포인트에서 좀비를 소환한다.
	int32 Count = SpawnPointArray.Num();
	if (Count > 0)
	{
		UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
		for (int32 i = 0; i < Count; ++i)
		{
			SpawnPointArray[i]->SpawnZombieByBossTrigger(Target);
		}
	}

	//쿨타임 선적용 어빌리티를 적용한다.
	FGameplayEventData EventData;
	EventData.Target = this;
	EventData.Instigator = this;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill6EarlyCooldown"));
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);

	if (HasAuthority())
	{
		//좀비 소환 : 6번 스킬
		PlayBossSkillSound(6);
	}
}

void AZombieBoss_Behemoth::Skill6End()
{
	Super::Skill6End();
}

bool AZombieBoss_Behemoth::Skill4Prepared()
{
	if (!bSkill4MoveStart)
	{
		UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
		if (IsValid(BB))
		{
			//돌을 던지러 가는 동작도 공격 중에 포함시켜야 한다
			//그래야 돌을 집으러 가는 행동이 캔슬되지 않는다.
			BB->SetValueAsBool(TEXT("Attacking"), true);
		}
		//돌을 들 수 있는 포지션으로 이동시킨다.
		//현재 위치에서 가장 가까운 픽업 좌표를 구한다.
		FVector CurrentLoc = GetActorLocation();
		//FVector Destination = PickupPointArray[0];
		FinishLoc = PickupPointArray[0];
		double Dist = FVector::DistSquared(CurrentLoc, FinishLoc);

		int32 Count = PickupPointArray.Num();
		for (int32 i = 1; i < Count; ++i)
		{
			double CurrentDist = FVector::DistSquared(CurrentLoc, PickupPointArray[i]);
			if (Dist > CurrentDist)
			{
				Dist = CurrentDist;
				FinishLoc = PickupPointArray[i];
			}
		}

		//해당 좌표로 이동시킨다.
		AICtrl->MoveToLocation(FinishLoc, -1.f, false, true, false);
		ChangeAnim(EZombieAnimType::Run);

		//다시 들어오지 않도록 false로 바꾼다.
		bSkill4MoveStart = true;
		return false;
	}
	else
	{
		EPathFollowingStatus::Type MoveResult = AICtrl->GetMoveStatus();
		//이동에 성공했다면 그때 true를 반환한다
		if (MoveResult == EPathFollowingStatus::Idle)
		{
			bSkill4MoveStart = false;
			//돌을 들어올리는 동안에는 이동을 하지 못하도록 한다.
			if (Movement)
			{
				Movement->StopMovementImmediately();
				Movement->Deactivate();
			}

			//돌을 뽑아드는 사운드를 재생한다.
			PlayBossSkillSound(4);

			return true;
		}
		else
		{
			return false;
		}
	}
}

void AZombieBoss_Behemoth::Multicast_LeapAttackEnd_Implementation()
{
	AnimInstance->BehemothLeapAttackEndSection();
}

void AZombieBoss_Behemoth::Multicast_StartThrowing_Implementation()
{
	AnimInstance->BehemothThrowingStoneSection();
}

void AZombieBoss_Behemoth::SetLandingLoc()
{
	//실제 도약 직전에 착지 좌표를 결정한다.
	UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();
	if (IsValid(BB))
	{
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));
		if (IsValid(Target))
		{
			FVector TargetForward = Target->GetActorForwardVector();
			FVector TargetCurrentLoc = Target->GetActorLocation();
			TargetLoc = TargetCurrentLoc + TargetForward * LandingOffset;
		}
	}
	MyLoc = GetActorLocation();

	//좌표가 정해졌으니 점프상태에 진입한다.
	bIsJumping = true;
}

void AZombieBoss_Behemoth::ApplyLandingAttack()
{
	//실제 리프 어택 어빌리티가 발동하는 지점
	//자신 주변으로 원형 영역을 충돌 체크한다.
	AAIController* Ctrl = GetController<AAIController>();
	if (IsValid(Ctrl))
	{
		//좀비의 공격 캡슐을 생성할 위치와 각도를 정한다.
		TArray<FHitResult> HitArray;
		FVector Loc = GetActorLocation();
		Loc.Z -= GetHalfHeight();

		//나 자신은 무시하도록 파라미터를 만든다.
		FCollisionQueryParams Param(NAME_None, false, this);

		//캡슐 모양으로 임시 충돌체를 생성해서 범위 충돌을 체크한다.
		bool Collision = GetWorld()->SweepMultiByChannel(HitArray, Loc, Loc, GetActorQuat(),
			ECollisionChannel::ECC_GameTraceChannel6, FCollisionShape::MakeSphere(400.f), Param);

		FColor DebugColor = FColor::Green;

		FGameplayEventData EventData;
		EventData.Instigator = this;
		EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill5"));

		if (HitArray.Num() > 0)
		{
			for (auto Hit : HitArray)
			{
				APlayerCharacterBase* HitActor = Cast<APlayerCharacterBase>(Hit.GetActor());
				if (!IsValid(HitActor))
				{
					continue;
				}

				DebugColor = FColor::Red;

				FGameplayAbilityTargetData_SingleTargetHit* TargetData =
					new FGameplayAbilityTargetData_SingleTargetHit(Hit);

				EventData.TargetData.Add(TargetData);
			}
		}

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);

#if WITH_EDITOR
		DrawDebugSphere(GetWorld(), Loc, 400.f, 32, DebugColor, false, 3.f);
#endif

	}

	if (HasAuthority())
	{
		//리프 어택 : 5번 스킬
		PlayBossSkillSound(5);
		PlayBossSkillEffect(5);
	}
}

void AZombieBoss_Behemoth::LaunchProjectile()
{
	//돌을 타겟을 향해 발사한다.
	//돌이 좀비의 손을 떠났으므로 돌의 위치 계산은 중단한다.
	bIsRaiseStone = false;

	//돌을 던졌으므로 이제 하체도 몽타주를 재생해야 한다.
	bIsOnlyUpperPlay = false;

	UProjectileMovementComponent* ProjMC = SpawnedStone->GetProjMovement();
	UBlackboardComponent* BB = AICtrl->GetBlackboardComponent();

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("Target")));

	FVector StartLoc = SpawnedStone->GetActorLocation();

	FVector EndLoc = FVector::ZeroVector;
	if (IsValid(Target))
	{
		//공격 모션이 시작되었을 때 타겟이 유효하다면 그 위치를
		EndLoc = Target->GetActorLocation();
	}
	else
	{
		//유효하지 않다면 일정 거리 앞의 지점을 목표로 잡는다.
		EndLoc = GetActorLocation() + GetActorForwardVector() * 1000.f;
	}
	FVector ResultVector;

	FCollisionResponseParams CRParam;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	UGameplayStatics::FSuggestProjectileVelocityParameters SPParam(GetWorld(), StartLoc, EndLoc, 3000.f);
	SPParam.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
	SPParam.bFavorHighArc = false;
	SPParam.CollisionRadius = 0.f;
	SPParam.OverrideGravityZ = 0.f;
	SPParam.ResponseParam = CRParam;
	SPParam.ActorsToIgnore = ActorsToIgnore;
	SPParam.bDrawDebug = true;

	bool CalcPath = UGameplayStatics::SuggestProjectileVelocity(SPParam, ResultVector);

	if (CalcPath)
	{
		ProjMC->Velocity = ResultVector;
	}
	else
	{
		FVector DirectDir = (EndLoc - StartLoc).GetSafeNormal();
		ProjMC->Velocity = DirectDir * 1500.f;
	}

	//프로젝타일 무브먼트 활성화
	ProjMC->Activate(true);

	//돌을 던지면 이때부터 쿨타임 태그 적용
	FGameplayEventData EventData;
	EventData.Target = this;
	EventData.Instigator = this;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill4EarlyCooldown"));
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventData.EventTag, EventData);
}

void AZombieBoss_Behemoth::SpawnStone()
{
	//보스 좀비의 양 손 사이에 돌을 스폰시킨다.
	//돌을 들어올릴 예정이므로 드는 중 변수를 활성화한다.
	bIsRaiseStone = true;

	//양 손 사이에 돌을 스폰시킨다.
	FVector RHand = Mesh->GetBoneLocation(TEXT("RightHand"));
	FVector LHand = Mesh->GetBoneLocation(TEXT("LeftHand"));
	FVector SpawnLoc = (RHand + LHand) * 0.5f;

	FRotator Rot = GetActorRotation();

	FActorSpawnParameters	param;
	param.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	param.Owner = this;
	param.Instigator = this;

	if (ThrowingStonProjectile)
	{
		//돌을 이동시켜주어야 하므로 멤버 변수로 백업한다.
		SpawnedStone = GetWorld()->SpawnActor<AZombieProjectileBase>(ThrowingStonProjectile, SpawnLoc, Rot, param);
		SpawnedStone->GetProjMovement()->bAutoActivate = false;
		SpawnedStone->SetOwnerController(GetController());
	}
}

bool AZombieBoss_Behemoth::CanUseMeleeAttack_Implementation()
{
	return true;
}

bool AZombieBoss_Behemoth::CanUseMiddleAttack_Implementation()
{
	//현재 중거리 공격 카테고리의 모든 행동이 불가능한 경우
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill1Cooldown"))) &&
		ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill5Cooldown"))))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool AZombieBoss_Behemoth::CanUseRangeAttack_Implementation()
{
	//현재 원거리 공격 카테고리의 모든 행동이 불가능한 경우
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(TEXT("Zombie.Battle.BZSkill4Cooldown"))))
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool AZombieBoss_Behemoth::CheckOnlyUpperPlay_Implementation()
{
	return bIsOnlyUpperPlay;
}
