// Written by EXPx2


#include "GameModes/URGameGameMode.h"

#include "URGame.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "Character/URPlayerCharacter.h"
#include "DataAssets/PlayerCharactersSet.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameInstances/URGameInstance.h"
#include "GameStates/URGameGameState.h"
#include "Player/URGamePlayerController.h"
#include "Player/URGamePlayerState.h"

AURGameGameMode::AURGameGameMode()
{
	// Lobby에서 캐릭터 전달 받아 시작
	bStartPlayersAsSpectators = true;
}

void AURGameGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	
	// 게임 종료됨
	if (ErrorMessage.IsEmpty())
	{
		if (AURGameGameState* GameGS = GetGameState<AURGameGameState>())
		{
			if (GameGS->IsCleared())
			{
				ErrorMessage = FString(TEXT("GameCleared"));
			}
		}
	}
}

APlayerController* AURGameGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal,
	const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	// 게임 종료되었으면 리턴
	if (AURGameGameState* GameGS = GetGameState<AURGameGameState>())
	{
		if (GameGS->IsCleared())
		{
			ErrorMessage = FString(TEXT("GameCleared"));
			return nullptr;
		}
	}
	
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}


bool AURGameGameMode::CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget)
{
	if (!IsValid(ViewTarget) ||
		ViewTarget->IsActorBeingDestroyed() || 
		!IsValid(ViewTarget->GetOwningController()) || 
		ViewTarget->GetOwningController()->IsActorBeingDestroyed())
	{
		return false;
	}
	
	if (AURGamePlayerState* GamePS = Cast<AURGamePlayerState>(ViewTarget))
	{
		if (!GamePS->GetPawn() || 
			!GamePS->GetURAbilitySystemComponent()->HasAvatarActor() ||
			GamePS->IsDead())
		{
			return false;
		}
	}
	
	return Super::CanSpectate_Implementation(Viewer, ViewTarget);
}

UClass* AURGameGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (UURGameInstance* URGI = GetGameInstance<UURGameInstance>())
	{
		if (AURGamePlayerState* GamePS = InController->GetPlayerState<AURGamePlayerState>())
		{
			if (UClass* CharacterClass = URGI->GetCharacterClass(GamePS->PlayingCharacterCode))
			{
				return CharacterClass;
			}
		}
	}
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}


bool AURGameGameMode::IsAllPlayersDead() const
{
	for (APlayerState* State : GameState->PlayerArray)
	{
		if (State->IsActorBeingDestroyed() || 
			!IsValid(State->GetOwningController()) || 
			State->GetOwningController()->IsActorBeingDestroyed())
		{
			// 접속 끊은 플레이어
			continue;
		}
		
		if (const AURGamePlayerState* GamePS = Cast<AURGamePlayerState>(State))
		{
			if (GamePS->GetPawn() &&
				GamePS->GetURAbilitySystemComponent()->HasAvatarActor() &&
				!GamePS->IsDead())
			{
				// 살아있는 플레이어 있음
				return false;
			}
		}
	}
	
	return true;
}

void AURGameGameMode::UpdateIsAllPlayersDead()
{
	bool bIsAllPlayersDead = IsAllPlayersDead();
	SetIsAllPlayersDead(bIsAllPlayersDead);
	
	if (!bIsAllPlayersDead)
	{
		if (RespawnTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
			RespawnTimerHandle.Invalidate();
		}
	}
}

void AURGameGameMode::OnPlayerDied(AURPlayerCharacter* PlayerCharacter)
{
	if (!ensure(GameState) || !IsValid(PlayerCharacter))
	{
		return;
	}
	
	if (AURGameGameState* GameGS = GetGameState<AURGameGameState>())
	{
		if (GameGS->IsCleared())
		{
			// 이미 클리어했을 경우 별도 처리하지 않음
			return;
		}
	}
	
	bool bIsAllPlayersDead = IsAllPlayersDead();
	
	SetIsAllPlayersDead(bIsAllPlayersDead);
	
	UR_SERVERLOG(LogUR, Log, TEXT("PlayersDied IsAll? %s"), bIsAllPlayersDead ? TEXT("true") : TEXT("false"));
	
	if (bIsAllPlayersDead)
	{
		// 모든 플레이어 사망
		for (APlayerState* State : GameState->PlayerArray)
		{
			if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(State->GetPlayerController()))
			{
				// 클라이언트 RPC
				GamePC->ClientOnAllPlayersDied();
			}
		}
		
		OnAllPlayersDied();
		return;
	}
	
	// 살아있는 플레이어 있음
	for (APlayerState* State : GameState->PlayerArray)
	{
		// 사망하여 관전중인 플레이어
		if (State->IsSpectator())
		{
			if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(State->GetPlayerController()))
			{
				if (GamePC->GetViewTargetPawn() == PlayerCharacter)
				{
					// 사망한 캐릭터를 관전 중이었을 경우 다음 플레이어 관전하도록 자동으로 넘어감 
					GamePC->ServerViewNextPlayer();
				}
			}
		}
	}
}

void AURGameGameMode::OnBossDied(AURBossCharacter* BossCharacter)
{
	if (GameClearTimerHandle.IsValid())
	{
		// 중복 요청 방지
		return;
	}

	if (AURGameGameState* GameGS = GetGameState<AURGameGameState>())
	{
		if (GameGS->IsCleared())
		{
			// 이미 클리어했을 경우 별도 처리하지 않음
			return;
		}
		
		GameGS->SetIsCleared(true);
	}
	
	// 리스폰 취소
	if (RespawnTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(RespawnTimerHandle);
		RespawnTimerHandle.Invalidate();
	}
	
	GetWorld()->GetTimerManager().SetTimer(GameClearTimerHandle, this, &ThisClass::ClearGame, 3.0f, false);
}

void AURGameGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);
	
	// 선택한 캐릭터로 생성하도록 캐릭터 코드 요청 
	if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(NewPlayer))
	{
		GamePC->InitializeCharacterCode();
	}
}

void AURGameGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	UpdateIsAllPlayersDead();
}

void AURGameGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);
	
	if (IsValid(NewPlayer->GetPawn()))
	{
		// 성공적으로 Restart한 경우 PlayerState에 Notify
		if (AURGamePlayerState* GamePS = NewPlayer->GetPlayerState<AURGamePlayerState>())
		{
			GamePS->ClientOnRespawn();
		}
	}
}

void AURGameGameMode::SetIsAllPlayersDead(const bool bInIsAllPlayersDead)
{
	// 게임 스테이트에 값 저장
	if (AURGameGameState* GameGS = GetGameState<AURGameGameState>())
	{
		GameGS->SetIsAllPlayersDead(bInIsAllPlayersDead);
	}
}

void AURGameGameMode::RespawnAllPlayers()
{
	if (AURGameGameState* GameGS = GetGameState<AURGameGameState>())
	{
		if (GameGS->IsCleared())
		{
			// 이미 클리어했을 경우 별도 처리하지 않음
			return;
		}
	}
	
	// 리스폰
	for (APlayerState* State : GameState->PlayerArray)
	{
		if (AURGamePlayerState* GamePS = Cast<AURGamePlayerState>(State))
		{
			if (GamePS->IsDead())
			{
				if (APlayerController* PC = State->GetPlayerController())
				{
					if (APawn* Pawn = PC->GetPawn())
					{
						// 기존 폰이 남아있을 경우 제거
						PC->UnPossess();
						Pawn->Destroy();
					}
		
					PC->Reset();
		
					RestartPlayer(PC);
				}
			}
		}
	}
	
	RespawnTimerHandle.Invalidate();
}

void AURGameGameMode::ClearGame()
{
	for (APlayerState* State : GameState->PlayerArray)
	{
		if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(State->GetPlayerController()))
		{
			// 클라이언트 RPC
			GamePC->ClientClearGame();
		}
	}
}

void AURGameGameMode::OnAllPlayersDied()
{
	if (RespawnTimerHandle.IsValid())
	{
		// 중복 요청 방지
		return;
	}
	
	if (AURGameGameState* GameGS = GetGameState<AURGameGameState>())
	{
		if (GameGS->IsCleared())
		{
			// 이미 클리어했을 경우 리스폰 X
			return;
		}
	}
	
	// 리스폰 타이머
	GetWorld()->GetTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::RespawnAllPlayers, 5.0f, false);
}
