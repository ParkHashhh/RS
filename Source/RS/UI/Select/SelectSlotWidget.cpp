// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectSlotWidget.h"
#include "RS/Player/Actor/SelectPawn.h"
#include "RS/RSUtils.h"
#include "RS/GameMode/RSGameInstance.h"
#include "RS/Player/SelectPlayerController.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/LogCategory.h"
#include "Kismet/GameplayStatics.h"
#include "RS/Data/Subsystem/DataStorageSubsystem.h"
#include "RS/GameMode/SessionSubsystem.h"

USelectSlotWidget::USelectSlotWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}


void USelectSlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TextureLeftButton->OnClicked.AddDynamic(this, &USelectSlotWidget::TextureLeftButtonClick);
	TextureRightButton->OnClicked.AddDynamic(this, &USelectSlotWidget::TextureRightClick);

	SkillLeftButton->OnClicked.AddDynamic(this, &USelectSlotWidget::SkillLeftButtonClick);
	SkillRightButton->OnClicked.AddDynamic(this, &USelectSlotWidget::SkillRightButtonClick);

	StartButton->OnClicked.AddDynamic(this, &USelectSlotWidget::StartButtonClick);
	MultiStartButton->OnClicked.AddDynamic(this, &USelectSlotWidget::MultiStartButtonClick);

}

void USelectSlotWidget::TextureLeftButtonClick()
{

	if (IsValid(CurrentSelectPawn))
	{
		CurrentSelectPawn->PrevTexture();
		TextureText->SetText(FText::FromName(CurrentSelectPawn->GetCurrentTextureName()));
	}
}

void USelectSlotWidget::TextureRightClick()
{

	if (IsValid(CurrentSelectPawn))
	{
		CurrentSelectPawn->NextTexture();
		TextureText->SetText(FText::FromName(CurrentSelectPawn->GetCurrentTextureName()));
	}
}

void USelectSlotWidget::SkillLeftButtonClick()
{
	if (IsValid(CurrentSelectPawn))
	{
		CurrentSelectPawn->PrevSkill();
		SkillText->SetText(FText::FromName((CurrentSelectPawn->GetCurrentSkillName())));
	}
}

void USelectSlotWidget::SkillRightButtonClick()
{
	if (IsValid(CurrentSelectPawn))
	{
		CurrentSelectPawn->NextSkill();
		SkillText->SetText(FText::FromName((CurrentSelectPawn->GetCurrentSkillName())));
	}
}
void USelectSlotWidget::StartButtonClick()
{
	if (!IsValid(CurrentSelectPawn))
	{
		return;
	}

	FName TextureName = CurrentSelectPawn->GetCurrentTextureName();
	FName SkillName = CurrentSelectPawn->GetCurrentSkillName();

	if (URSGameInstance* GI = GetGameInstance<URSGameInstance>())
	{
		if (auto* Storage = GI->GetSubsystem<UDataStorageSubsystem>())
		{
			Storage->SetSelectedCharacter(TextureName, SkillName);
		}

		GI->LevelLoading(TEXT("Colony"), TEXT("/Game/Levels/Colony"),TEXT(""),true,false, true);
	}
}
void USelectSlotWidget::MultiStartButtonClick()
{
	RS_PRINT_SCREEN(1000.f, FColor::Red, TEXT("MultiStartButtonClick : Session"));

	if (!IsValid(CurrentSelectPawn))
	{
		return;
	}

	FName TextureName = CurrentSelectPawn->GetCurrentTextureName();
	FName SkillName = CurrentSelectPawn->GetCurrentSkillName();

	if (URSGameInstance* GI = GetGameInstance<URSGameInstance>())
	{
		if (auto* Storage = GI->GetSubsystem<UDataStorageSubsystem>())
		{
			Storage->SetSelectedCharacter(TextureName, SkillName);
		}

		if (auto* SessionSubsystem = GI->GetSubsystem<USessionSubsystem>())
		{
			SessionSubsystem->CreateSession();
		}
	}
}
void USelectSlotWidget::PickSelectPawn(ASelectPawn* SelectPawn)
{
	// 여기서 텍스쳐 이름 출력
	CurrentSelectPawn = SelectPawn;
	if (IsValid(CurrentSelectPawn))
	{
		if (SkillText)
		{
			SkillText->SetText(FText::FromName(CurrentSelectPawn->GetCurrentSkillName()));
		}
		if (TextureText)
		{
			TextureText->SetText(FText::FromName(CurrentSelectPawn->GetCurrentTextureName()));
		}
	}
}

void USelectSlotWidget::TestJoinLocalHost()
{
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FName TextureName = CurrentSelectPawn->GetCurrentTextureName();
		FName SkillName = CurrentSelectPawn->GetCurrentSkillName();

		FString URL = FString::Printf(TEXT("listen?Port=7777?Texture=%s Skill=%s "),
			*TextureName.ToString(), *SkillName.ToString());

		if (URSGameInstance* GI = GetGameInstance<URSGameInstance>())
		{
			RSUtils::OpenLevelWithDataStorage(GetWorld(), TEXT("Colony"), true, true, URL);
		}

		RS_PRINT_SCREEN(1000.f, FColor::Red, TEXT("URL : %s"),*URL);
	}
}
