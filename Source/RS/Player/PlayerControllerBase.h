	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/RSPlayerController.h"
#include "RS/UI/Battle/EndPopUpWidget.h"

#include "PlayerControllerBase.generated.h"

class APlayerCharacterBase;
class URewardPopupWidget;
class UBattleHUDWidget;
class UPauseWidget;
class USettingWidget;
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
	TArray<APlayerCharacterBase*> SpectateTargets;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> EndPopUpWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> EndPopUpWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPauseWidget> PauseWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<USettingWidget> SettingWidgetClass;

	int32 CurrentSpectateIndex = 0;

public:
	// 서버: 관전 목록 계산 요청
	UFUNCTION(Server, Reliable)
	void Server_RequestSpectateTargets();

	// 클라이언트: 계산된 목록 받기
	UFUNCTION(Client, Reliable)
	void Client_ReceiveSpectateTargets(const TArray<APlayerCharacterBase*>& Targets);

	// 입력 바인딩용
	void SpectateNext();
	void SpectatePrev();

public:
	const FString& GetPlayerName() const
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
	virtual void ApplyGameplayInputMode();
private:
	void CreateBattleHUD();
	void CreateEndPopUp();

#pragma region Interaction

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<URewardPopupWidget> RewardPopupWidgetClass;

	UPROPERTY(Transient)
	TWeakObjectPtr<URewardPopupWidget> RewardPopupWidget;

public:
	UFUNCTION(Client, Reliable)
	void Client_ShowRewardUI(const TArray<FName>& RewardItemRowNames);

	UFUNCTION(Server, Reliable)
	void Server_RequestReward(FName RewardItemRowName);

	void CloseRewardPopup();

private:
	void CreateRewardPopupWidget(const TArray<FName>& RewardItemRowNames);
	void CreatePauseWidget();
	void CreateSettingWidget();
#pragma endregion Interaction
};
