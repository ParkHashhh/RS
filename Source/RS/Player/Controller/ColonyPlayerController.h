// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RS/Player/Controller/PlayerControllerBase.h"
#include "GameFramework/PlayerController.h"
#include "RS/Core/RSTypes.h"
#include "ColonyPlayerController.generated.h"

class UStageResultWidget;

/**
 *
 */
UCLASS()
class RS_API AColonyPlayerController : public APlayerControllerBase
{
	GENERATED_BODY()

public:
	AColonyPlayerController();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> ColonyWidgetClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> ColonyWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MapSelectWidgetClass;
	UPROPERTY()

	TObjectPtr<UUserWidget> MapSelectWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UStageResultWidget> StageResultWidgetClass;

	UPROPERTY()
	TObjectPtr<UStageResultWidget> ActiveStageResultWidget;

	// Client_ShowStageResultWidget RPC로 함께 실려오는 값 - ARSPlayerState의 KillCount/PlayTime은
	// OnRep 없는 평범한 리플리케이트 프로퍼티라 이 RPC보다 늦게 도착할 수 있어서, 값 자체를 RPC 파라미터로 전달받아 보관한다.
	int32 PendingStageResultKillCount = 0;
	float PendingStageResultPlayTime = 0.f;
	TArray<FItemStorageInfo> PendingStageResultNewItems;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Sound")
	TObjectPtr<USoundBase> MapSelectOpenSound;
	UPROPERTY(EditDefaultsOnly, Category = "UI|Sound")
	TObjectPtr<USoundBase> MapSelectCloseSound;

public:
	TObjectPtr<UUserWidget> GetColonyWidget()
	{
		return ColonyWidget;
	}

	void ShowMapSelectWidget();
	void RemoveMapSelectWidget();
	void ShowStageResultWidget();

	int32 GetPendingStageResultKillCount() const
	{
		return PendingStageResultKillCount;
	}
	float GetPendingStageResultPlayTime() const
	{
		return PendingStageResultPlayTime;
	}
	const TArray<FItemStorageInfo>& GetPendingStageResultNewItems() const
	{
		return PendingStageResultNewItems;
	}

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* aPawn) override;
	virtual void ApplyGameplayInputMode() override;

public:
	UFUNCTION(Server, Reliable)
	void Server_RequestPortal(FName LevelName, const FString& PackagePath);

	UFUNCTION(Client, Reliable)
	void Client_ShowStageResultWidget(int32 InKillCount, float InPlayTime, const TArray<FItemStorageInfo>& InNewItems);

	UFUNCTION(Server, Reliable)
	void Server_ClearPendingStageResult();

	UFUNCTION(Server, Reliable)
	void Server_SetSelectedCharacterInfo(FName TextureName, FName SkillName);
};
