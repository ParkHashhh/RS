// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RS/RSUtils.h"
#include "RS/Utils/ASCProvider.h"
#include "RSPlayerState.generated.h"

class UInventoryComponent;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE(FOnPlayerStateReadyDelegate);

/**
 *
 */
UCLASS()
class RS_API ARSPlayerState : public APlayerState, public IASCProvider
{
	GENERATED_BODY()

	inline static const FName InventoryComponentName = FName(TEXT("Inventory"));


public:
	ARSPlayerState();

private:
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_IsPlayerStateReady, meta=(AllowPrivateAccess=true))
	bool bPlayerStateReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, BlueprintGetter=K2_GetInventoryComponent, Category="Inventory", meta=(AllowPrivateAccess=true))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	FOnPlayerStateReadyDelegate OnPlayerStateReady;

private:
	UPROPERTY(Replicated)
	FName SelectedTextureName;
	UPROPERTY(Replicated)
	FName SelectedSkillName;
public:
	virtual void CopyProperties(APlayerState* PlayerState) override;
public:
	void SetTextureName(const FName& Name)
	{
		SelectedTextureName = Name;
	}

	void SetSkillName(const FName& Name)
	{
		SelectedSkillName = Name;
	}
	FName GetTextureName() const
	{
		return SelectedTextureName;
	}

	FName GetSkillName() const
	{
		return SelectedSkillName;
	}
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	template <typename T>
	void ExecuteOrBindOnPlayerStateReady(T* Object, void(T::*Callback)())
	{
		RSUtils::ExecuteOrBind(IsPlayerStateReady(), OnPlayerStateReady, Object, Callback);
	}

	template <typename T>
	void RemoveAllOnPlayerStateReady(T* Object)
	{
		static_assert(TIsDerivedFrom<T, UObject>::IsDerived, "T must be derived from UObject");
		OnPlayerStateReady.RemoveAll(Object);
	}

	FORCEINLINE bool IsPlayerStateReady() const
	{
		return bPlayerStateReady;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Is Player State Ready")
	bool K2_IsPlayerStateReady() const
	{
		return IsPlayerStateReady();
	}

	void NotifyPlayerStateReady();

	virtual UAbilitySystemComponent* GetASC() const override;

	FORCEINLINE UInventoryComponent* GetInventoryComponent() const
	{
		return InventoryComponent;
	}

	UFUNCTION(BlueprintGetter, DisplayName="Get Inventory Component")
	UInventoryComponent* K2_GetInventoryComponent() const
	{
		return GetInventoryComponent();
	}

protected:
	virtual void BeginPlay() override;

private:
	void InitializePlayerState();

#pragma region StageResult
private:
	int32 KillCount;
	float PlayTime;

public:
	int32 GetKillCount() const
	{
		return KillCount;
	}

	void SetKillCount(int32 InKillCount)
	{
		this->KillCount = InKillCount;
	}

	float GetPlayTime() const
	{
		return PlayTime;
	}

	void SetPlayTime(float InPlayTime)
	{
		this->PlayTime = InPlayTime;
	}
#pragma endregion
};
