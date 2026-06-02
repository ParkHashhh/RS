// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryComponent.generated.h"

class UDataSubsystem;
struct FItemTableInfo;

USTRUCT(BlueprintType)
struct FInventoryItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemRowName;

	UPROPERTY(BlueprintReadOnly)
	int32 Count{1};

	// 장착 해제 시 효과를 꺼주기 위해 GAS가 발급해 준 핸들을 보관 (로컬/서버 인스턴스 관리용)
	UPROPERTY()
	FActiveGameplayEffectHandle AppliedEffectHandle;

	[[nodiscard]] const FItemTableInfo* GetRowInfo(const UDataSubsystem* Subsystem) const;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RS_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UInventoryComponent();

	// 소유자 캐릭터의 ASC를 이용해 아이템 효과를 적용하는 함수 예시
	UFUNCTION(BlueprintCallable, Category = "Inventory|GAS")
	void EquipItem(UPARAM(ref) FInventoryItem& Item, UAbilitySystemComponent* TargetASC);

	UFUNCTION(BlueprintCallable, Category = "Inventory|GAS")
	void UnEquipItem(UPARAM(ref) FInventoryItem& Item, UAbilitySystemComponent* TargetASC);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
};
