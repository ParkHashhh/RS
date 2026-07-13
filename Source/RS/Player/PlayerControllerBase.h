	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/RSPlayerController.h"
#include "RS/UI/Battle/EndPopUpWidget.h"

#include "PlayerControllerBase.generated.h"

class UBattleHUDWidget;
/**
 *
 */
UCLASS()
class RS_API APlayerControllerBase : public ARSPlayerController
{
	GENERATED_BODY()

public:
	APlayerControllerBase();

protected:
	FString PlayerName;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UBattleHUDWidget> BattleHUDWidgetClass;

	UPROPERTY()
	TArray<class APlayerCharacterBase*> SpectateTargets;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> EndPopUpWidgetClass;

	TObjectPtr<UUserWidget> EndPopUpWidget;

	int32 CurrentSpectateIndex = 0;

	FName SelectedTextureName;

	FName SelectedSkillName;


public:
	void SetTextureName(const FName& TextureName)
	{
		SelectedTextureName = TextureName;
	}

	void SetSkillName(const FName& SkillName)
	{
		SelectedSkillName = SkillName;
	}

	FName GetTextureName()
	{
		return SelectedTextureName;
	}

	FName GetSkillName()
	{
		return SelectedSkillName;
	}
	// 서버: 관전 목록 계산 요청
	UFUNCTION(Server, Reliable)
	void Server_RequestSpectateTargets();

	// 클라이언트: 계산된 목록 받기
	UFUNCTION(Client, Reliable)
	void Client_ReceiveSpectateTargets(const TArray<class APlayerCharacterBase*>& Targets);

	// 입력 바인딩용
	void SpectateNext();
	void SpectatePrev();

public:
	FORCEINLINE FString GetPlayerName() const
	{
		return PlayerName;
	}

	void SetPlayerName(const FString& NewPlayerName)
	{
		PlayerName = NewPlayerName;
	}

	void ShowEndPopUp();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	void CreateBattleHUD();
	void CreateEndPopUp();

#pragma region Interaction

public:

	UFUNCTION(Client, Reliable)
	void Client_ShowRewardUI(const TArray<FName>& RewardItemRowNames);

#pragma endregion Interaction
};
