	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/Controller/RSPlayerController.h"
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

	//생성된 통합 HUD 위젯을 보관할 멤버 변수
	UPROPERTY()
	TObjectPtr<UBattleHUDWidget> BackupBattleHUDWidget;

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

	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TSubclassOf<class URSUserWidget> ControlHelpWidgetClass;

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

	UBattleHUDWidget* GetBattleHUDWidget() const
	{
		return BackupBattleHUDWidget;
	}

	void ShowEndPopUp();

	// 디버그용 콘솔 명령: 콘솔창에 "SetDifficultyStage 10" 입력하면 바로 해당 난이도 스테이지로 세팅됨
	UFUNCTION(Exec)
	void SetDifficultyStage(int32 NewStage);

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
	UFUNCTION(Server, Reliable)
	void Server_SyncSelectedCharacterInfo(FName TextureName, FName SkillName);

	// Server_RequestBattleRestart를 BeginPlay에서 한 번만 부르면, 그 시점에 서버가 아직
	// HasClientLoadedCurrentWorld()를 false로 보고 있을 때(웨어하우스처럼 로딩이 오래 걸리는 맵)
	// 재시도 없이 그대로 폰이 안 생긴 채로 끝나버림. 폰이 생길 때까지 주기적으로 재시도한다.
	FTimerHandle RequestBattleRestartRetryHandle;
	void RetryRequestBattleRestart();
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

	UFUNCTION(Server, Reliable)
	void Server_RequestBattleRestart();
	void CloseRewardPopup();
	UFUNCTION(Client, Reliable)
	void Client_CloseRewardPopup();
private:
	void CreateRewardPopupWidget(const TArray<FName>& RewardItemRowNames);
	void CreatePauseWidget();
	void CreateSettingWidget();
	void CreateControlHelpWidget();
#pragma endregion Interaction
};
