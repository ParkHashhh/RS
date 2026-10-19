// Fill out your copyright notice in the Description page of Project Settings.

#include "RS/Player/Controller/SelectPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "RS/Player/Character/SelectPawn.h"
#include "Blueprint/UserWidget.h"
#include "RS/UI/Select/SelectUserWidget.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Core/LogCategory.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"

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

	GetWorldTimerManager().SetTimerForNextTick(this, &ASelectPlayerController::FindMainCamera);

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

void ASelectPlayerController::FindMainCamera()
{
	TArray<AActor*> TaggedCameras;
	UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), ACameraActor::StaticClass(), FName("MainCamera"), TaggedCameras);
	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| TaggedCameras.Num()=%d"), *LOG_LINE_INFO, TaggedCameras.Num());

	if (TaggedCameras.Num() > 0)
	{
		SelectCamera = Cast<ACameraActor>(TaggedCameras[0]);
	}

	if (SelectCamera)
	{
		SetViewTarget(SelectCamera);
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
	// UDataStorageSubsystem::SelectedTextureName은 GameInstance당 하나뿐인 전역 값이라, 리슨 서버에서
	// 원격 플레이어의 RPC가 이걸 건드리면 호스트가(같은 GameInstance를 읽는) 자기 걸로 착각해서 씀 -
	// 호스트 자신의 로컬 커넥션일 때만 이 공유 필드를 갱신
	if (IsLocalController())
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				StorageSubsystem->SetSelectedCharacter(TextureName, SkillName);
			}
		}
	}

	RSUtils::ServerTravelWithDataStorage(GetWorld(), TEXT("Colony"), true);
}
