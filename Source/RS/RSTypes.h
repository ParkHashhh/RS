#pragma once
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "PaperSprite.h"

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
struct FStatBase : public FTableRowBase
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
struct FItemTableInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPaperSprite* IconSprite;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsStackable{true};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemUseType UseType{EItemUseType::Equipment};

	// 모든 아이템 스탯 처리를 일괄 담당할 마스터 GE 클래스 레퍼런스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|GAS")
	TSubclassOf<UGameplayEffect> MasterItemEffectClass;

	// 기획자가 3~5개 등 원하는 만큼 자유롭게 넣을 수 있는 아이템 옵션 리스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|GAS")
	TArray<FItemOption> ItemOptions;
};

USTRUCT(BlueprintType)
struct FMiscTableInfo : public FTableRowBase
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
struct FPlayerDefaultInfo : public FTableRowBase
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
	float AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Damage")
	float DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Stat|Shot")
	int32 Ammo{};
};

UENUM(BlueprintType)
enum class ENormalZombieRarity : uint8
{
	Common,
	Rare
};

USTRUCT(BlueprintType)
struct FZombieDefaultInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|HP")
	float MaxHP{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Move")
	float WalkSpeed{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Move")
	float RunSpeed{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float AttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Range")
	float AttackDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Detect")
	float DetectRange_Sight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Type")
	float Normal_Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Cooldown")
	float Special_SkillCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float Special_SkillDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zombie|Stat|Damage")
	float Special_SkillDotDamage;
};

UENUM(BlueprintType)
enum class EZombieAnimType : uint8
{
	Idle,
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
struct FStatDisplayTableInfo : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FText DisplayName;

    UPROPERTY(EditAnywhere)
    FText DescriptionFormat;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UPaperSprite> Icon;

	// 값이 0이면 UI에서 숨길지 여부
    UPROPERTY(EditAnywhere)
    bool bHiddenIfZero = true;
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
struct FStageObjective
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage|Objective")
	EClearCondition ClearCondition;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stage|Objective")
	int32 Current;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage|Objective")
	int32 Goal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage|Objective")
	TArray<FDataTableRowHandle> Targets;

	bool IsCleared() const
	{
		return Current >= Goal;
	}

	float GetProgressRatio() const
	{
		return Goal > 0 ? FMath::Clamp(static_cast<float>(Current) / Goal, 0.0f, 1.0f) : 1.0f;
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
		return Lhs.ClearCondition == RHS.ClearCondition
			&& Lhs.Current == RHS.Current
			&& Lhs.Goal == RHS.Goal
			&& Lhs.Targets == RHS.Targets;
	}

	friend bool operator!=(const FStageObjective& Lhs, const FStageObjective& RHS)
	{
		return !(Lhs == RHS);
	}

	FStageObjective() = default;

	FStageObjective(EClearCondition ClearCondition, int32 Current, int32 Goal)
		: ClearCondition(ClearCondition),
		  Current(Current),
		  Goal(Goal)
	{
	}
};
