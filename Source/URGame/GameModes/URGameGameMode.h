// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Character/URBossCharacter.h"
#include "GameFramework/GameModeBase.h"
#include "URGameGameMode.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API AURGameGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AURGameGameMode();
	
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	
	virtual bool CanSpectate_Implementation(APlayerController* Viewer, APlayerState* ViewTarget) override;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	
	UFUNCTION(BlueprintPure, Category="UR")
	bool IsAllPlayersDead() const;
	
	/** 모든 플레이어가 죽었는지 여부를 GameState에 갱신한다. */
	void UpdateIsAllPlayersDead();
	
	void OnPlayerDied(class AURPlayerCharacter* PlayerCharacter);
	
	void OnBossDied(AURBossCharacter* BossCharacter);


protected:
	virtual void OnPostLogin(AController* NewPlayer) override;
	
	virtual void BeginPlay() override;
	
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	
	void SetIsAllPlayersDead(bool bInIsAllPlayersDead);
	
	UFUNCTION()
	void RespawnAllPlayers();
	
	void ClearGame();
	
	void OnAllPlayersDied();
	
	FTimerHandle RespawnTimerHandle;
	
	FTimerHandle GameClearTimerHandle;
};
