// Fill out your copyright notice in the Description page of Project Settings.


#include "RS/GameFramework/GameMode/ColonyGameMode.h"
#include "RS/Player/RSPlayerState.h"
#include "RS/Core/LogCategory.h"
#include "RS/GameFramework/Subsystem/DataStorageSubsystem.h"
#include "RS/Player/Controller/ColonyPlayerController.h"
#include "RS/GameFramework/RSGameInstance.h"
#include "RS/GameFramework/GameState/RSGameState.h"

AColonyGameMode::AColonyGameMode()
{
	bUseSeamlessTravel = true;
	bStartPlayersAsSpectators = true;
}

void AColonyGameMode::InitGameState()
{
	Super::InitGameState();

	// 콜로니 GameState는 스테이지에서 돌아올 때마다 새로 생성돼서 DifficultyStage가 매번 기본값(1)로
	// 리셋됨 - GameInstance에 저장해둔 값을 여기서 복원해야 맵선택 화면의 "Level" 표시가(리플리케이트되는
	// GameState 프로퍼티를 읽으므로 원격 클라 포함) 보스를 잡은 뒤에도 올바르게 유지됨.
	if (auto* GS = GetGameState<ARSGameState>())
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				StorageSubsystem->RestoreDifficultyStage(GS);
			}
		}
	}
}

void AColonyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| PostLogin: %s"), *LOG_LINE_INFO, *GetNameSafe(NewPlayer));
	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| PostLogin: %s"), *LOG_LINE_INFO, *GetNameSafe(NewPlayer));
}

void AColonyGameMode::OnPlayerCharacterSelected(AColonyPlayerController* PlayerController, FName TextureName,
	FName SkillName)
{
	if (!IsValid(PlayerController))
	{
		return;
	}

	auto* PS = PlayerController->GetPlayerState<ARSPlayerState>();
	if (!IsValid(PS))
	{
		return;
	}

	PS->SetTextureName(TextureName);
	PS->SetSkillName(SkillName);

	if (APawn* OldPawn = PlayerController->GetPawn())
	{
		OldPawn->Destroy();
	}

	RS_LOG_DEBUG(LogRSSession, Log, TEXT("%s| Character Selected: %s, %s"), *LOG_LINE_INFO, *TextureName.ToString(), *SkillName.ToString());

	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s| Pawn Before=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));
	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| Pawn Before=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));

	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| Controller Local=%s"), *LOG_LINE_INFO, PlayerController->IsLocalController()? TEXT("true") : TEXT("false"));

	RestartPlayer(PlayerController);

	RS_LOG_DEBUG(LogRSCommon, Log, TEXT("%s| Pawn After=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));
	RS_PRINT_SCREEN(30.0f, FColor::Cyan, TEXT("%s| Pawn After=%s"), *LOG_LINE_INFO, *GetNameSafe(PlayerController->GetPawn()));
}

APlayerController* AColonyGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
                                          const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	APlayerController* Result = Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
	if (!Result)
	{
		return nullptr;
	}

	RS_PRINT_SCREEN(5, FColor::Red, TEXT("Login called! Options: [%s]"), *Options);
	RS_PRINT_SCREEN(5, FColor::Red, TEXT("HasAuthority %d"), HasAuthority());

	FString TextureName;
	FString SkillName;

	// 언리얼 URL은 "?Texture=A?Skill=B?Name=C"처럼 옵션마다 공백 없이 ?로만 구분되는데,
	// FParse::Value는 공백에서만 멈추고 ?에서는 안 멈춰서 뒤에 붙은 옵션까지 통째로 값으로 먹어버림 -
	// 직접 ?로 쪼개서 파싱해야 "Texture=SKM_..._Mesh3?Skill=Heal?Name=..." 같은 오염을 막을 수 있음
	TArray<FString> OptionParts;
	Options.ParseIntoArray(OptionParts, TEXT("?"), true);
	for (const FString& Part : OptionParts)
	{
		if (Part.StartsWith(TEXT("Texture=")))
		{
			TextureName = Part.Mid(8);
		}
		else if (Part.StartsWith(TEXT("Skill=")))
		{
			SkillName = Part.Mid(6);
		}
	}
	if (ARSPlayerState* PS = Result->GetPlayerState<ARSPlayerState>())
	{
		if (!TextureName.IsEmpty())
		{
			PS->SetTextureName(FName(TextureName));
		}
		if (!SkillName.IsEmpty())
		{
			PS->SetSkillName(FName(SkillName));
		}
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (auto* StorageSubsystem = GI->GetSubsystem<UDataStorageSubsystem>())
			{
				// UDataStorageSubsystem::SelectedTextureName은 GameInstance당 하나뿐인 전역 값이라
				// 리슨 서버에서는 호스트 자신의 GameInstance = 서버 GameInstance라서, 원격 플레이어의
				// Login()에서 이걸 건드리면 호스트가(같은 GameInstance를 읽는) 그 값을 자기 걸로 착각해서 씀.
				// 호스트 자신의 로컬 커넥션일 때만 이 공유 필드를 갱신
				if (Result->IsLocalController() && (!TextureName.IsEmpty() || !SkillName.IsEmpty()))
				{
					StorageSubsystem->SetSelectedCharacter(
						!TextureName.IsEmpty() ? FName(TextureName) : StorageSubsystem->GetSelectedTextureName(),
						!SkillName.IsEmpty() ? FName(SkillName) : StorageSubsystem->GetSelectedSkillName());
				}

				// 포탈로 스테이지 클리어 후 돌아올 때는 ServerTravel이라 Options가 아예 비어있어서
				// "Options 없으면 복원" 조건으로는 못 막음 - PlayerState에 이미 유효한 텍스쳐가
				// 있는지(seamless travel의 CopyProperties로 이미 채워졌는지)로 판단해야 모든 경로에서 안전함
				if (PS->GetTextureName().IsNone())
				{
					StorageSubsystem->RestoreTextureSkill(PS);
				}
			}
		}
	}

	return Result;
}
