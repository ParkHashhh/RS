#pragma once
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "PaperSprite.h"
#include "Net/Serialization/FastArraySerializer.h"

struct FStageObjectiveEntry;
class ARSGameState;
struct FStageObjectiveList;
class UDataSubsystem;
class UGameplayEffect;

#include "RSTypes.generated.h"

UENUM(BlueprintType)
enum class EItemUseType : uint8
{
	Equipment,
	Consumable,
};

UENUM(BlueprintType)
enum class EGrade : uint8
{
	Common,
	Rare,
	Epic,
	Legendary,
};

// 기획자가 에디터/엑셀에서 입력할 동적 옵션 구조체
USTRUCT(BlueprintType)
struct FItemOption
{
	GENERATED_BODY()

	// 예: SetByCaller.Stat.AttackPower, SetByCaller.Stat.CriticalChance 등
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Option")
	FGameplayTag OptionTag{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Option")
	float Value{0.f};

	// 이단점프 해금, 자동조준 해금 등의 옵션이면 false, stack에 따라 증가하는 옵션이면 true
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Option")
	bool bScaleWithStack{true};
};

USTRUCT(BlueprintType)
struct RS_API FStatBase : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseAttackPower{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ArmorPenetration{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FixedDamageReduction{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RatioDamageReduction{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WalkSpeed{0.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RunSpeed{0.f};
};


USTRUCT(BlueprintType)
struct RS_API FItemTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> IconTexture{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UPaperSprite> IconSprite{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsStackable{true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemUseType UseType{EItemUseType::Equipment};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EGrade Grade{EGrade::Common};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight{0.f};

	// 이 아이템이 드롭 후보로 등장하기 시작하는 최소 스테이지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinDropStage{1};

	// 기획자가 3~5개 등 원하는 만큼 자유롭게 넣을 수 있는 아이템 옵션 리스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|GAS")
	TArray<FItemOption> ItemOptions{};
};

USTRUCT(BlueprintType)
struct RS_API FMiscTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	FLinearColor CommonColor{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	FLinearColor RareColor{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	FLinearColor EpicColor{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	FLinearColor LegendaryColor{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	float CommonIconThickness{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	float RareIconThickness{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	float EpicIconThickness{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Misc|UI")
	float LegendaryIconThickness{};
};

USTRUCT(BlueprintType)
struct RS_API FPlayerDefaultInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float AimShotSpreadRadius{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float BaseShotSpreadRadius{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float FireShotSpreadModifier{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|HP")
	float MaxHP{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Range")
	float MaxRange{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Move")
	float WalkSpeed{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Move")
	float DashSpeed{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Damage")
	float AttackPower{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Damage")
	float DefensePower{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	int32 Ammo{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float FireBulletChancePercent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float FireBulletDamagePercent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float ElectricBulletChancePercent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float ElectricBulletDamagePercent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float IceBulletChancePercent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float IceBulletDamagePercent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float PoisonBulletChancePercent{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	float PoisonBulletDamagePercent{};
};

UENUM(BlueprintType)
enum class ENormalZombieRarity : uint8
{
	Common,
	Rare
};

USTRUCT(BlueprintType)
struct RS_API FZombieDefaultInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|HP")
	float MaxHP{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Move")
	float WalkSpeed{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Move")
	float RunSpeed{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float AttackPower{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float DefensePower{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Range")
	float AttackDistance{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Detect")
	float DetectRange_Sight{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Detect")
	float DetectRange_Hearing{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|CheckBoss")
	bool bIsBoss{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Type")
	float Normal_Rarity{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float Special_SkillCooldown{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float Special_SkillDamage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float Special_SkillDotDamage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float	Boss_Skill1Damage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float	Boss_Skill1Cooldown{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float	Boss_Skill2Damage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float	Boss_Skill2Cooldown{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float	Boss_Skill3Damage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float	Boss_Skill3Cooldown{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float	Boss_Skill4Damage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float	Boss_Skill4Cooldown{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float	Boss_Skill5Damage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float	Boss_Skill5Cooldown{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float	Boss_Skill6Damage{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float	Boss_Skill6Cooldown{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Range")
	float	Boss_MeleeAttackDistance{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Range")
	float	Boss_MiddleAttackDistance{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Range")
	float	Boss_RangeAttackDistance{};
};

//보스의 스킬은 1~7이 되도록 위치를 고정한다.
UENUM(BlueprintType)
enum class EZombieAnimType : uint8
{
	Idle,
	Boss_Skill1,
	Boss_Skill2,
	Boss_Skill3,
	Boss_Skill4,
	Boss_Skill5,
	Boss_Skill6,
	CombatIdle,
	Walk,
	Run,
	Attack,
	Roar,
	Special_Skill,
	Jump,
	Death
};

struct FAttributeDelegateBinding
{
	FGameplayAttribute Attribute;
	FDelegateHandle DelegateHandle;
};

UENUM(BlueprintType)
enum class ERSInputID : uint8
{
	None,
	Confirm,
	Cancel,
	Fire,
	Aim,
	Reload,
	SubSkill,
	Dash,
	Jump,
	Interact,
};

USTRUCT(BlueprintType)
struct RS_API FStatDisplayTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FText DisplayName{};

	UPROPERTY(EditAnywhere)
	FText DescriptionFormat{};

	UPROPERTY(EditAnywhere)
	TObjectPtr<UPaperSprite> Icon{};

	// 값이 0이면 UI에서 숨길지 여부
	UPROPERTY(EditAnywhere)
	bool bHiddenIfZero{true};
};

UENUM(BlueprintType)
enum class EClearCondition : uint8
{
	None = 0,
	KillBoss,
	KillEnemy,
	SurviveTime,
	Escape,
	GetItem,
};

USTRUCT(BlueprintType)
struct RS_API FStageObjectiveTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EClearCondition ClearCondition{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Goal{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ObjectiveName{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ObjectiveDescription{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TargetTableRowName{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bScaleGoalByPlayerCount = false;

	// KillEnemy는 인원수만큼 곱연산, SurviveTime은 인원당 15초씩 가산. 그 외/플래그 꺼짐이면 원본 Goal 그대로.
	float GetScaledGoal(int32 PlayerCount) const
	{
		if (!bScaleGoalByPlayerCount)
		{
			return Goal;
		}
		if (ClearCondition == EClearCondition::KillEnemy)
		{
			return Goal * PlayerCount;
		}
		if (ClearCondition == EClearCondition::SurviveTime)
		{
			return Goal + (15.f * (PlayerCount - 1));
		}
		return Goal;
	}
};

USTRUCT(BlueprintType)
struct FStageObjective
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Objective")
	FName TableRowName{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Objective")
	float Current{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Objective")
	float Goal{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Objective")
	TArray<FDataTableRowHandle> Targets{};

	bool IsValid() const
	{
		return !TableRowName.IsNone();
	}

	bool IsCleared() const
	{
		return Current >= Goal;
	}

	static int32 GetGoal(FName RowName, const UDataSubsystem* DataSubsystem);

	float GetProgressRatio() const
	{
		return Goal > 0 ? FMath::Clamp(Current / Goal, 0.0f, 1.0f) : 1.0f;
	}

	void Reset()
	{
		Current = 0.0f;
	}

	int32 AddProgress(int32 Amount = 1)
	{
		Current += Amount;
		return Current;
	}

	friend bool operator==(const FStageObjective& Lhs, const FStageObjective& RHS)
	{
		return Lhs.TableRowName == RHS.TableRowName
			&& Lhs.Current == RHS.Current
			&& Lhs.Goal == RHS.Goal
			&& Lhs.Targets == RHS.Targets;
	}

	friend bool operator!=(const FStageObjective& Lhs, const FStageObjective& RHS)
	{
		return !(Lhs == RHS);
	}

	FStageObjective() = default;

	FStageObjective(FName TableRowName, float Current, float Goal)
		: TableRowName(TableRowName),
		  Current(Current),
		  Goal(Goal)
	{
	}

	FStageObjective(const FStageObjectiveEntry& Entry, const UDataSubsystem* DataSubsystem);
};

UENUM(BlueprintType)
enum class EInteractableState : uint8
{
	None = 0,
	Idle,
	Occupied,
	Disabled,
};

USTRUCT(BlueprintType)
struct RS_API FStageTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> ObjectiveRowNames{};
};

USTRUCT(BlueprintType)
struct RS_API FKillInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FUniqueNetIdRepl UniqueNetId{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MonsterRowName{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 KillCount{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsBoss{}; 
};

USTRUCT(BlueprintType)
struct RS_API FStageObjectiveEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FName TableRowName{};

	UPROPERTY()
	float CurreentProgress{};

	UPROPERTY()
	int32 DisplayOrder{};

	FStageObjectiveEntry() = default;
	FStageObjectiveEntry(FName TableRowName, float CurrentProgress, int32 DisplayOrder)
		: TableRowName(TableRowName)
		, CurreentProgress(CurrentProgress)
		, DisplayOrder(DisplayOrder)
	{
	}

	void PreReplicatedRemove(const FStageObjectiveList& InArraySerializer);
	void PostReplicatedAdd(const FStageObjectiveList& InArraySerializer);
	void PostReplicatedChange(const FStageObjectiveList& InArraySerializer);

	bool IsValid() const
	{
		return !TableRowName.IsNone();
	}

	friend bool operator==(const FStageObjectiveEntry& Lhs, const FStageObjectiveEntry& RHS)
	{
		return Lhs.TableRowName == RHS.TableRowName;
	}

	friend bool operator!=(const FStageObjectiveEntry& Lhs, const FStageObjectiveEntry& RHS)
	{
		return !(Lhs == RHS);
	}
};

USTRUCT()
struct RS_API FStageObjectiveList : public FFastArraySerializer
{
	GENERATED_BODY()

	friend class ARSGameState;

	UPROPERTY()
	TArray<FStageObjectiveEntry> Entries{};

	UPROPERTY(NotReplicated)
	TObjectPtr<ARSGameState> Owner{};

private:
	UPROPERTY(NotReplicated)
	TMap<FName, int32> TableRowNameIndexMap{};

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FStageObjectiveEntry, FStageObjectiveList>(Entries, DeltaParams, *this);
	}

	void MarkDirty(FName TableRowName);

	FStageObjectiveEntry* FindEntry(FName TableRowName);
	const FStageObjectiveEntry* FindEntry(FName TableRowName) const;

	FStageObjectiveEntry& AddEntry(FName TableRowName, float CurrentProgress = 0.0f, int32 DisplayOrder = 0);

	bool RemoveEntry(FName TableRowName);

private:
	void RebuildIndexMap();
};

// NetDeltaSerializer 활성화를 위한 특수화 등록
template <>
struct TStructOpsTypeTraits<FStageObjectiveList> : public TStructOpsTypeTraitsBase2<FStageObjectiveList>
{
	enum { WithNetDeltaSerializer = true };
};

UENUM(BlueprintType)
enum class EStageResultStatType : uint8
{
	None,
	Kill,
	Time,
};

USTRUCT()
struct RS_API FItemStorageInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FName RowName{};

	UPROPERTY()
	int32 Count{};
};

USTRUCT()
struct RS_API FStageResultStatInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int32 KillCount{};

	UPROPERTY()
	float PlayTime{};
};
