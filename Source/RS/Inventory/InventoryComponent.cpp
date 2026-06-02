// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

#include "AbilitySystemComponent.h"
#include "RS/LogCategory.h"
#include "RS/RSTypes.h"
#include "RS/Data/Subsystem/DataSubsystem.h"


const FItemTableInfo* FInventoryItem::GetRowInfo(const UDataSubsystem* Subsystem) const
{
	if (!IsValid(Subsystem) || ItemRowName.IsNone())
	{
		return nullptr;
	}

	return Subsystem->GetItemTableInfo(ItemRowName);
}

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UInventoryComponent::EquipItem(FInventoryItem& Item, UAbilitySystemComponent* TargetASC)
{
	if (!IsValid(TargetASC))
	{
		return;
	}

	const UWorld* World = GetWorld();
	check(World);

	const UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);

	const auto* DataSubsystem = GameInstance->GetSubsystem<UDataSubsystem>();
	check(DataSubsystem);

	const FItemTableInfo* ItemTableInfo = Item.GetRowInfo(DataSubsystem);
	if (!ItemTableInfo || !ItemTableInfo->MasterItemEffectClass)
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Invalid item or missing effect class"), *LOG_LINE_INFO);
		return;
	}

	FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetOwner());

	const FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
		ItemTableInfo->MasterItemEffectClass, 1.f, EffectContext);

	if (SpecHandle.IsValid())
	{
		// 데이터 테이블에 정의된 n개의 옵션을 순회하며 SetByCaller 값 주입
		for (const auto& [OptionTag, Value] : ItemTableInfo->ItemOptions)
		{
			if (OptionTag.IsValid())
			{
				SpecHandle.Data->SetSetByCallerMagnitude(OptionTag, Value);
			}
		}

		// 캐릭터에게 최종 빌드된 아이템 효과 적용 및 핸들 저장
		Item.AppliedEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	}
}

void UInventoryComponent::UnEquipItem(FInventoryItem& Item, UAbilitySystemComponent* TargetASC)
{
	if (!IsValid(TargetASC) || !Item.AppliedEffectHandle.IsValid())
	{
		RS_LOG_DEBUG(LogRSData, Warning, TEXT("%s: Invalid ability system component or applied effect handle"), *LOG_LINE_INFO);
		return;
	}

	TargetASC->RemoveActiveGameplayEffect(Item.AppliedEffectHandle);
	Item.AppliedEffectHandle = {};
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
