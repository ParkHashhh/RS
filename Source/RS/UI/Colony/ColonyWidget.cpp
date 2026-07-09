// Fill out your copyright notice in the Description page of Project Settings.


#include "ColonyWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"       
#include "RS/LogCategory.h"


UColonyWidget::UColonyWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UColonyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	StageButton->OnClicked.AddDynamic(this, &UColonyWidget::StageClick);
}

void UColonyWidget::StageClick()
{
	RS_PRINT_SCREEN(5.f, FColor::Red, TEXT("StageClick"));
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Stage_Warehouse"));
}
