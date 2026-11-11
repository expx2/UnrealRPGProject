// Written by EXPx2


#include "GameStates/URGameGameState.h"

#include "URGame.h"
#include "Net/UnrealNetwork.h"

void AURGameGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(AURGameGameState, bIsAllPlayersDead, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(AURGameGameState, bIsCleared, COND_None, REPNOTIFY_OnChanged);
}

void AURGameGameState::SetIsAllPlayersDead(const bool bInIsAllPlayersDead)
{
	if (bIsAllPlayersDead != bInIsAllPlayersDead)
	{
		bIsAllPlayersDead = bInIsAllPlayersDead;
		OnIsAllPlayersDeadValueChanged.Broadcast(bIsAllPlayersDead);
	}
}

void AURGameGameState::SetIsCleared(const bool bInIsCleared)
{
	bIsCleared = bInIsCleared;
}

void AURGameGameState::OnRep_bIsAllPlayersDead()
{
	OnIsAllPlayersDeadValueChanged.Broadcast(bIsAllPlayersDead);
}

void AURGameGameState::OnRep_bIsCleared()
{
	
}
