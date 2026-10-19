// Fill out your copyright notice in the Description page of Project Settings.


#include "RewardPopupWidget.h"

#include "Components/RewardEntryObject.h"
#include "Components/TileView.h"
#include "RS/Core/LogCategory.h"
#include "RS/Data/Subsystem/DataSubsystem.h"

URewardPopupWidget::URewardPopupWidget(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void URewardPopupWidget::Init(const TArray<FName>& RewardItemRowNames)
{
	ensureMsgf(!RewardItemRowNames.IsEmpty() && RewardItemRowNames.Num() <= MaxRewardCount,
	           TEXT("%s: RewardItemRowNames cannot be empty and should not exceed MaxRewardCount. Num(): %d"),
	           *LOG_LINE_INFO, RewardItemRowNames.Num());

	RewardEntryObjects.Reset(RewardItemRowNames.Num());
	RewardTileView->ClearListItems();

	auto* DataSubsystem = GetSubsystem<UDataSubsystem>();
	if (!DataSubsystem)
	{
		return;
	}

	for (const FName RewardItemRowName : RewardItemRowNames)
	{
		if (RewardItemRowName.IsNone())
		{
			continue;
		}

		auto* NewObj = NewObject<URewardEntryObject>(this);
		NewObj->Initialize(RewardItemRowName, DataSubsystem);

		RewardEntryObjects.Emplace(NewObj);
	}

	RewardTileView->SetListItems(RewardEntryObjects);
}

void URewardPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = true;

		// 기본값(bHideCursorDuringCapture=true, LockOnCapture)이면 첫 클릭이 뷰포트 마우스 캡처에
		// 소모되어 커서가 숨겨지고, 위젯에는 두 번째 클릭부터 전달됨 - 인벤토리 팝업과 동일한 원인
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
}

void URewardPopupWidget::NativeDestruct()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->bShowMouseCursor = false;

		PC->SetInputMode(FInputModeGameOnly());
	}

	Super::NativeDestruct();
}
