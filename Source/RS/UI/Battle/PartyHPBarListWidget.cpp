// Fill out your copyright notice in the Description page of Project Settings.


#include "PartyHPBarListWidget.h"
#include "RS/GameMode/GameplaySubsystem.h"
#include "RS/GameMode/RSGameState.h"
#include "GameFramework/PlayerState.h"
#include "RS/UI/Battle/PartyHPBarWidget.h"
#include "RS/Player/PlayerCharacterBase.h"

UPartyHPBarListWidget::UPartyHPBarListWidget(const FObjectInitializer& ObjectInitializer)
	: URSUserWidget(ObjectInitializer)
{
}

void UPartyHPBarListWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UPartyHPBarListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//바인딩한 체력바들로 배열을 만든다
	HPBarArray.Empty();
	HPBarArray.Add(Row1);
	HPBarArray.Add(Row2);
	HPBarArray.Add(Row3);

	//준비가 완료되었다는 신호가 오면 파티원 체력바를 한번 갱신해준다.
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (auto* GameplaySubsystem = LP->GetSubsystem<UGameplaySubsystem>())
		{
			if (GameplaySubsystem->IsGameplayReady())
			{
				SetPartyList();
			}
		}
	}

	//인원 변동 시 호출되도록 한 델리게이트에 갱신 함수를 바인딩한다.
	if (GetWorld())
	{
		ARSGameState* GS = GetWorld()->GetGameState<ARSGameState>();
		if (GS)
		{
			GS->OnPartyListChangeDelegate.AddUObject(this, &UPartyHPBarListWidget::SetPartyList);
		}
	}
}

void UPartyHPBarListWidget::NativeDestruct()
{
	//게임스테이트에 연결한 델리게이트를 해제한다.
	if (GetWorld())
	{
		ARSGameState* GS = GetWorld()->GetGameState<ARSGameState>();
		if (GS)
		{
			GS->OnPartyListChangeDelegate.RemoveAll(this);
		}
	}

	Super::NativeDestruct();
}

void UPartyHPBarListWidget::SetVisibility(ESlateVisibility InVisibility)
{
	//화면에 출력하는 옵션인 경우 현재 활성화된 체력바 위젯 개수를 체크해야 한다.
	if (InVisibility == ESlateVisibility::Visible ||
		InVisibility == ESlateVisibility::HitTestInvisible ||
		InVisibility == ESlateVisibility::SelfHitTestInvisible)
	{
		if (ActivateHPBarCount == 0)
		{
			//만약 0개라면 비활성화를 해야 한다.
			SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Super::SetVisibility(InVisibility);
		}
	}
	else
	{
		Super::SetVisibility(InVisibility);
	}
}

void UPartyHPBarListWidget::SetPartyList()
{
	//게임스테이트로부터 플레이어 배열을 얻어온다.
	if (!GetWorld())
	{
		return;
	}

	ARSGameState* GS = GetWorld()->GetGameState<ARSGameState>();
	if (!GS)
	{
		return;
	}
	TArray<TObjectPtr<APlayerState>> PlayerStateArray = GS->PlayerArray;

	//'나' 를 구분하기 위해 내 컨트롤러를 얻어온다.
	APlayerController* MyCtrl = GetOwningPlayer();

	//나를 뺀 나머지만 모아 배열을 만든다.
	TArray<TObjectPtr<APlayerState>> ArrayExcludeMe = {};
	for (TObjectPtr<APlayerState> PState : PlayerStateArray)
	{

		if (!PState || PState->GetOwner() == MyCtrl)
		{
			continue;
		}
		ArrayExcludeMe.Add(PState);
	}

	//체력창에는 체력바가 3개 있으므로 이를 기준으로 반복문을 돌린다.
	int32 HPBarCount = HPBarArray.Num();
	for (int32 i = 0; i < HPBarCount; ++i)
	{
		//그 인덱스가 플레이어 배열에서도 유효하다면 플레이어 정보를 체력바에게 넘겨준다.
		if (ArrayExcludeMe.IsValidIndex(i))
		{
			HPBarArray[i]->SetVisibility(ESlateVisibility::Visible);
			APlayerCharacterBase* PartyMember = ArrayExcludeMe[i]->GetPawn<APlayerCharacterBase>();
			if (!PartyMember)
			{
				HPBarArray[i]->SetVisibility(ESlateVisibility::Collapsed);
				continue;
			}
			HPBarArray[i]->SetPlayer(PartyMember, i + 1);
		}
		else
		{
			HPBarArray[i]->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	//활성화된 체력바 위젯의 개수를 멤버 변수에 넣어준다.
	int32 Count = 0;
	for (auto HPBar : HPBarArray)
	{
		if (HPBar->GetVisibility() == ESlateVisibility::Visible)
		{
			++Count;
		}
	}
	ActivateHPBarCount = Count;

	//인원수 변동을 고려해 파티원 리스트 위젯의 출력을 재설정한다.
	SetVisibility(GetVisibility());
}
