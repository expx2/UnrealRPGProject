// Written by EXPx2


#include "Character/UREnemyCharacter.h"

#include "URAggroComponent.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/URCombatSet.h"
#include "AbilitySystem/AttributeSet/URHealthSet.h"
#include "AI/URAIController.h"
#include "Definitions/URFaction.h"

AUREnemyCharacter::AUREnemyCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	URASC = CreateDefaultSubobject<UURAbilitySystemComponent>(TEXT("UR ASC"));
	URASC->SetIsReplicated(true);
	URASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AggroComponent = CreateDefaultSubobject<UURAggroComponent>(TEXT("Aggro Component"));
	
	HealthSet = CreateDefaultSubobject<UURHealthSet>(TEXT("Health Set"));
	CombatSet = CreateDefaultSubobject<UURCombatSet>(TEXT("Combat Set"));
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	static ConstructorHelpers::FClassFinder<AAIController> AIControllerRef(TEXT("/Game/_ProjectUR/AI/BP_AIController"));
	if (AIControllerRef.Succeeded())
	{
		AIControllerClass = AIControllerRef.Class;
	}
}

void AUREnemyCharacter::ResetForSpawn()
{
	Super::ResetForSpawn();
	
	if (AggroComponent)
	{
		AggroComponent->ResetAggro();
	}
}
