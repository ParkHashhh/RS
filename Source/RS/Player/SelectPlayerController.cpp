// Fill out your copyright notice in the Description page of Project Settings.

#include "SelectPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "RS/Player/Actor/SelectPawn.h"
#include "Blueprint/UserWidget.h"
#include "RS/UI/Select/SelectUserWidget.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/LogCategory.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"

ASelectPlayerController::ASelectPlayerController()
{
	bShowMouseCursor = true;
	CurrentMouseCursor = EMouseCursor::Custom;
}

void ASelectPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ASelectPlayerController::HandleLeftClick);
}

void ASelectPlayerController::HandleLeftClick()
{
	FHitResult Hit;

	bool Pick = GetHitResultUnderCursor(ECollisionChannel::ECC_GameTraceChannel8, true, Hit);

	if (Pick)
	{
		ASelectPawn* SelectPawn = Cast<ASelectPawn>(Hit.GetActor());

		if (SelectPawn && IsInputKeyDown(EKeys::LeftMouseButton))
		{
			OnPawnSelected(SelectPawn);
		}
	}
}

void ASelectPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void ASelectPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameAndUI InputMode;
	SetInputMode(InputMode);

	TArray<AActor*> TaggedCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), FName("MainCamera"), TaggedCameras);

	if (SelectCamera)
	{
		SetViewTarget(SelectCamera);
	}


	if (IsValid(SelectWidgetClass))
	{
		SelectWidget = CreateWidget<USelectUserWidget>(GetWorld(), SelectWidgetClass);
		if (IsValid(SelectWidget))
		{
			CurrentMouseCursor = EMouseCursor::Custom;
			SelectWidget->AddToViewport();
			SelectWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

}

void ASelectPlayerController::OnPawnSelected(ASelectPawn* SelectPawn)
{

	if (CurrentSelectedPawn == SelectPawn)
	{
		CurrentSelectedPawn = nullptr;

		SetViewTargetWithBlend(SelectCamera, 0.5f);
		if (SelectWidget)
		{
			// 숨기기 처리
			SelectWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}
	if (CurrentSelectedPawn && CurrentSelectedPawn != SelectPawn)
	{
		return;
	}
	CurrentSelectedPawn = SelectPawn;

	SetViewTargetWithBlend(CurrentSelectedPawn->GetCamera(), 0.5f);

	// 텍스처 선택 위젯 띄우기
	if (!SelectWidget && SelectWidgetClass)
	{
		SelectWidget = CreateWidget<USelectUserWidget>(this, SelectWidgetClass);
	}
	// 위젯 동기화 해야함
	if (SelectWidget)
	{
		SelectWidget->PickSelectPawn(CurrentSelectedPawn);
		SelectWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ASelectPlayerController::Server_SetSelectedCharacterInfo_Implementation(FName TextureName, FName SkillName)
{
	ARSPlayerState* PS = GetPlayerState<ARSPlayerState>();
	if (PS)
	{
		PS->SetTextureName(TextureName);
		PS->SetSkillName(SkillName);
		UE_LOG(LogTemp, Warning, TEXT("Saved to PlayerState. Now GetTextureName: [%s]"), *PS->GetTextureName().ToString());
	}
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
		{
			StorageSubsystem->SetSelectedCharacter(TextureName, SkillName);
		}
	}

	RSUtils::ServerTravelWithDataStorage(GetWorld(), TEXT("Colony"), true);
}
