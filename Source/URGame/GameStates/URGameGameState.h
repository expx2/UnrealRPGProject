// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "URGameGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIsAllPlayersDeadValueChangedDelegate, bool, bValue);

/**
 * 
 */
UCLASS()
class URGAME_API AURGameGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetIsAllPlayersDead(const bool bInIsAllPlayersDead);
	
	void SetIsCleared(const bool bInIsCleared);
	
	bool IsAllPlayersDead() const { return bIsAllPlayersDead; }
	
	bool IsCleared() const { return bIsCleared; }
	
	UPROPERTY(BlueprintAssignable, Category="UR|Game")
	FIsAllPlayersDeadValueChangedDelegate OnIsAllPlayersDeadValueChanged;
	
protected:
	UFUNCTION()
	void OnRep_bIsAllPlayersDead();
	
	UFUNCTION()
	void OnRep_bIsCleared();
	
	/** 모든 플레이어가 죽었는지 여부 */
	UPROPERTY(BlueprintReadOnly, Category="UR|Game", ReplicatedUsing=OnRep_bIsAllPlayersDead)
	bool bIsAllPlayersDead = false;
	
	UPROPERTY(BlueprintReadOnly, Category="UR|Game", ReplicatedUsing=OnRep_bIsCleared)
	bool bIsCleared = false;
};
