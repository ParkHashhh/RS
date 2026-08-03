// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubSystems.h"
#include "EnhancedInputComponent.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "InputData.generated.h"

/**
 *
 */

USTRUCT(BlueprintType)
struct FInputKeyNames
{
	GENERATED_BODY()

	inline static const FName Move = FName(TEXT("Move"));
	inline static const FName Dash = FName(TEXT("Dash"));
	inline static const FName Attack = FName(TEXT("Attack"));
	inline static const FName Jump = FName(TEXT("Jump"));
	inline static const FName Rotation = FName(TEXT("Rotation"));
	inline static const FName Aim = FName(TEXT("Aim"));
	inline static const FName Reload = FName(TEXT("Reload"));
	inline static const FName SubSkill = FName(TEXT("SubSkill"));
	inline static const FName SpecNext = FName(TEXT("SpecNext"));
	inline static const FName SpecPrev = FName(TEXT("SpecPrev"));
	inline static const FName Inventory = FName(TEXT("Inventory"));
	inline static const FName Interaction = FName(TEXT("Interaction"));
	inline static const FName Cursor = FName(TEXT("Cursor"));
	inline static const FName Pause = FName(TEXT("Pause"));

	inline static const FName Debug_00 = FName(TEXT("debug_00"));
	inline static const FName Debug_01 = FName(TEXT("debug_01"));
	inline static const FName Debug_02 = FName(TEXT("debug_02"));
	inline static const FName Debug_03 = FName(TEXT("debug_03"));
	inline static const FName Debug_04 = FName(TEXT("debug_04"));
	inline static const FName Debug_05 = FName(TEXT("debug_05"));
	inline static const FName Debug_06 = FName(TEXT("debug_06"));
	inline static const FName Debug_07 = FName(TEXT("debug_07"));
	inline static const FName Debug_08 = FName(TEXT("debug_08"));
	inline static const FName Debug_09 = FName(TEXT("debug_09"));
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class RS_API UInputData : public UObject
{
	GENERATED_BODY()

public:
	UInputData();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> Context;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FName, TObjectPtr<UInputAction>> Actions;

	TObjectPtr<UInputAction> FindAction(const FName& Name) const;
};

UCLASS()
class RS_API UDefaultInputData : public UInputData
{
	GENERATED_BODY()

public:
	UDefaultInputData();
};


UCLASS()
class RS_API USelectInputData : public UInputData
{
	GENERATED_BODY()

public:
	USelectInputData();
};
