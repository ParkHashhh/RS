// Fill out your copyright notice in the Description page of Project Settings.


#include "PartyHPBarListWidget.h"
#include "RS/Player/Subsystem/GameplaySubsystem.h"
#include "RS/GameFramework/GameState/RSGameState.h"
#include "GameFramework/PlayerState.h"
#include "RS/UI/Battle/PartyHPBarWidget.h"
#include "RS/Player/Character/PlayerCharacterBase.h"
#include "RS/Core/LogCategory.h"

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
			GameplaySubsystem->OnGameplayReady.AddWeakLambda(this, [this](bool bReady)
				{
					if (bReady)
					{
						SetPartyList();
					}
				});

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
	//이 구문으로 제어를 하는 게 아니라 일종의 최소한의 방어 구문이라 생각해야 한다.
	if (InVisibility == ESlateVisibility::Visible ||
		InVisibility == ESlateVisibility::HitTestInvisible ||
		InVisibility == ESlateVisibility::SelfHitTestInvisible)
	{
		if (ActivateHPBarCount == 0)
		{
			//만약 0개라면 비활성화를 해야 한다.
			Super::SetVisibility(ESlateVisibility::Hidden);
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
		RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| GameState's PlayerArray get Failed."), *LOG_LINE_INFO);
		return;
	}
	TArray<TObjectPtr<APlayerState>> PlayerStateArray = GS->PlayerArray;


	APlayerController* MyCtrl = GetOwningPlayer();
	if (!IsValid(MyCtrl))
	{
		RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| Owner Controller get Failed."), *LOG_LINE_INFO);
		return;
	}

	APlayerState* MyPlayerState = MyCtrl->PlayerState;
	bool bAnyPartyMemberMissing = false;
	if (IsValid(MyPlayerState))
	{
		//나를 뺀 나머지만 모아 배열을 만든다.
		TArray<TObjectPtr<APlayerState>> ArrayExcludeMe = {};
		for (TObjectPtr<APlayerState> PState : PlayerStateArray)
		{

			//if (!PState || PState->GetOwner() == MyCtrl)
			if (!PState || PState == MyPlayerState)
			{
				continue;
			}
			ArrayExcludeMe.Add(PState);
		}

		RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| SetPartyList CALLED, PlayerArray: %d, ArrayExcludeMe: %d"),
			*LOG_LINE_INFO, PlayerStateArray.Num(), ArrayExcludeMe.Num());

		//체력창에는 체력바가 3개 있으므로 이를 기준으로 반복문을 돌린다.
		int32 HPBarCount = HPBarArray.Num();
		for (int32 i = 0; i < HPBarCount; ++i)
		{
			//그 인덱스가 플레이어 배열에서도 유효하다면 플레이어 정보를 체력바에게 넘겨준다.
			if (ArrayExcludeMe.IsValidIndex(i))
			{
				HPBarArray[i]->SetVisibility(ESlateVisibility::Visible);
				APlayerCharacterBase* PartyMember = ArrayExcludeMe[i]->GetPawn<APlayerCharacterBase>();
				RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| Index %d, PartyMember: %s"),
					*LOG_LINE_INFO, i, *GetNameSafe(PartyMember));
				if (!PartyMember)
				{
					// 상대방 폰이 아직 리플리케이션되기 전일 수 있음 - 다음 틱에 재시도
					bAnyPartyMemberMissing = true;
					HPBarArray[i]->SetVisibility(ESlateVisibility::Hidden);
					continue;
				}
				HPBarArray[i]->SetPlayer(PartyMember, i + 1);
			}
			else
			{
				HPBarArray[i]->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	else
	{
		bAnyPartyMemberMissing = true;
	}

	// 폰이 아직 리플리케이션 안 된 파티원이 있으면 1초 후 다시 시도한다.
	// 예전엔 최대 30틱(프레임 기준이라 저프레임/랙 구간에선 체감 시간이 훨씬 짧음)만 재시도하고
	// 영영 포기했는데, 그러면 그 파티원 접속/퇴장(OnPartyListChangeDelegate) 전까지 체력바가
	// 계속 숨겨진 채로 남았다. 횟수 제한 없이 계속 재시도하도록 바꿈 - 호출 비용이 낮아서
	// (플레이어 최대 인원 순회) 무한 재시도해도 부담 없다.
	if (bAnyPartyMemberMissing && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(SetPartyListRetryHandle, this,
			&UPartyHPBarListWidget::SetPartyList, 1.0f, false);
	}

	//활성화된 체력바 위젯의 개수를 멤버 변수에 넣어준다.
	int32 Count = 0;
	for (auto HPBar : HPBarArray)
	{
		if (HPBar->GetVisibility() == ESlateVisibility::Visible ||
			HPBar->GetVisibility() == ESlateVisibility::HitTestInvisible ||
			HPBar->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
		{
			++Count;
		}
	}
	ActivateHPBarCount = Count;

	RS_LOG_DEBUG(LogRSSession, Warning, TEXT("%s| Activated PartyHPBarWidget Number : %d"),
		*LOG_LINE_INFO, Count);

	//카운트가 1 이상이라면 파티원 체력창을 이곳에서 활성화시킨다.
	if (ActivateHPBarCount > 0)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}
