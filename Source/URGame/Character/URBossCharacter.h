// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Character/UREnemyCharacter.h"
#include "URBossCharacter.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API AURBossCharacter : public AUREnemyCharacter
{
	GENERATED_BODY()
	
protected:
	virtual void HandleDeath(AActor* Actor) override;
};
