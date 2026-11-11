// Written by EXPx2


#include "Character/URBossCharacter.h"

#include "GameModes/URGameGameMode.h"

void AURBossCharacter::HandleDeath(AActor* Actor)
{
	Super::HandleDeath(Actor);

	if (HasAuthority())
	{
		if (AURGameGameMode* GameMode = GetWorld()->GetAuthGameMode<AURGameGameMode>())
		{
			GameMode->OnBossDied(this);
		}
	}
}
