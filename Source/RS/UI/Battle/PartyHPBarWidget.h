// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/UI/Core/RSUserWidget.h"
#include "RS/Core/RSTypes.h"
#include "PartyHPBarWidget.generated.h"

class UTextBlock;
class UProgressBar;
struct FOnAttributeChangeData;
class APlayerCharacterBase;

UCLASS()
class RS_API UPartyHPBarWidget : public URSUserWidget
{
	GENERATED_BODY()

public:
	UPartyHPBarWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PartyMemberName;

	UPROPERTY(meta = (BindWidget, AllowPrivateAccess = true), BlueprintReadOnly, Category = "HPBar|Asset")
	TObjectPtr<UProgressBar> PartyMemberHPBar;

	float CurrHP = 0.0f;
	float MaxHP = 1.0f;

	//이 위젯이 담당할 플레이어
	UPROPERTY()
	TObjectPtr<APlayerCharacterBase> PartyMember;

	//이 위젯이 파티 리스트에 표시될 순서
	UPROPERTY()
	int32 ListOrder = 0;

protected:
	TArray<FAttributeDelegateBinding> AttributeDelegateBindings;

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "On HP Changed", Category = "HPBar")
	void K2_OnHPChanged(float HPRatio);

private:
	void OnCurrHpChanged(const FOnAttributeChangeData& Data);
	void OnMaxHpChanged(const FOnAttributeChangeData& Data);

private:
	void SetPartyMemberInfo();

public:
	void SetPlayer(APlayerCharacterBase* Player, int32 Order)
	{
		if (!Player)
		{
			return;
		}

		PartyMember = Player;

		ListOrder = Order;

		//플레이어가 세팅되었다면 정보를 변경해준다.
		SetPartyMemberInfo();
	}
};
