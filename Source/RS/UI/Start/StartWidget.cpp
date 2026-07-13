// Fill out your copyright notice in the Description page of Project Settings.


#include "StartWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"       
#include "RS/GameMode/RSGameInstance.h"


UStartWidget::UStartWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UStartWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	StartButton->OnClicked.AddDynamic(this, &UStartWidget::StartClick);
	ExitButton->OnClicked.AddDynamic(this, &UStartWidget::ExitClick);
}

void UStartWidget::StartClick()
{

	URSGameInstance* GI = GetGameInstance<URSGameInstance>();
	if (IsValid(GI))
	{
		GI->SetNextLevelName("Select");
	}

	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Loading"));
}

void UStartWidget::ExitClick()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
