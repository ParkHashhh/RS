// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "RS/LogCategory.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/GameMode/RSInGameMode.h"
#include "RS/RSUtils.h"
#include "Kismet/GameplayStatics.h"
#include "RS/UI/Colony/Result/StageResultWidget.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/Player/RSPlayerState.h"

AColonyPlayerController::AColonyPlayerController()
{
	bShowMouseCursor = true;
}


void AColonyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(ColonyWidgetClass))
	{
		ColonyWidget = CreateWidget<UUserWidget>(GetWorld(), ColonyWidgetClass);
		if (IsValid(ColonyWidget))
		{
			ColonyWidget->AddToViewport();

			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(ColonyWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			SetInputMode(InputMode);
		}
	}
	if (IsValid(MapSelectWidgetClass))
	{
		MapSelectWidget = CreateWidget<UUserWidget>(GetWorld(), MapSelectWidgetClass);
	}

	URSGameInstance* GI = Cast<URSGameInstance>(GetGameInstance());
	if (IsValid(GI))
	{
		GI->StopBGM();
	}
	
}

void AColonyPlayerController::ShowMapSelcetWidget()
{
	if (IsValid(MapSelectWidget))
	{
		bShowMouseCursor = true;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MapSelectWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);		
		UGameplayStatics::PlaySound2D(this, MapSelectOpenSound);
		MapSelectWidget->AddToViewport();
	}
}

void AColonyPlayerController::RemoveMapSelectWidget()
{
	if (IsValid(MapSelectWidget))
	{
		bShowMouseCursor = false;
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		UGameplayStatics::PlaySound2D(this, MapSelectCloseSound);
		MapSelectWidget->RemoveFromParent();
	}
}

// 1회성이기 때문에 멤버변수로 놓을 필요없음
void AColonyPlayerController::ShowStageResultWidget()
{
	if (!IsValid(StageResultWidgetClass))
	{
		RS_PRINT_SCREEN(100.f, FColor::Red, TEXT("ShowStageResultWidget: StageResultWidgetClass is NOT set"));
		return;
	}

	UStageResultWidget* Widget = CreateWidget<UStageResultWidget>(this, StageResultWidgetClass);
	if (!IsValid(Widget))
	{
		return;
	}


	bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	Widget->AddToViewport();

}

void AColonyPlayerController::Server_RequestPortal_Implementation(FName LevelName, const FString& PackagePath)
{
	ARSInGameMode* GM = GetWorld()->GetAuthGameMode<ARSInGameMode>();
	if (IsValid(GM))
	{
		GM->CreateOrUpdatePortal(LevelName, PackagePath);
	}
}

void AColonyPlayerController::Client_ShowStageResultWidget_Implementation()
{
	ShowStageResultWidget();
}

void AColonyPlayerController::Server_ClearPendingStageResult_Implementation()
{
	if (const ARSPlayerState* PS = GetPlayerState<ARSPlayerState>())
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				StorageSubsystem->ConsumePendingStageResult(PS->GetUniqueId());
			}
		}
	}
}
