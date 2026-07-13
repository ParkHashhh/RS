// Fill out your copyright notice in the Description page of Project Settings.


#include "SelectSlotWidget.h"
#include "RS/Player/Actor/SelectPawn.h"
#include "Kismet/GameplayStatics.h"       
#include "RS/LogCategory.h"
#include "RS/GameMode/RSGameInstance.h"

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

	if (URSGameInstance* GI = GetGameInstance<URSGameInstance>())
	{
		GI->SelectedTextureName = CurrentSelectPawn->GetCurrentTextureName();
		GI->SelectedSkillName = CurrentSelectPawn->GetCurrentSkillName();

	}
	// ? 가 언리얼 표준 규칙 
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Colony"));

}
void USelectSlotWidget::PickSelectPawn(ASelectPawn* SelectPawn)
{
	// 여기서 텍스쳐 이름 출력
	CurrentSelectPawn = SelectPawn;
	if (IsValid(CurrentSelectPawn))
	{
		if (SkillText)
		{
			SkillText->SetText(FText::FromName((CurrentSelectPawn->GetCurrentSkillName())));
		}
		if (TextureText)
		{
			TextureText->SetText(FText::FromName(CurrentSelectPawn->GetCurrentTextureName()));
		}
	}
}
