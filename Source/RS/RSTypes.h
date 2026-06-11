#pragma once
#include "GameplayTagContainer.h"

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
	FString Name{};

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Combat|Shot")
	float BaseShotSpreadRadius{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Combat|Shot")
	float AimShotSpreadModifier{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Combat|Shot")
	float MoveShotSpreadModifier{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Combat|Shot")
	float FireShotSpreadModifier{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Combat|Shot")
	float ShotSpreadRecoveryTime{};
};

UENUM(BlueprintType)
enum class EZombieAnimType : uint8
{
	Idle,
	Walk,
	Run,
	Attack,
	Death
};
