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

