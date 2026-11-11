// Written by EXPx2


#include "Player/URGamePlayerState.h"

#include "URGame.h"
#include "URGamePlayerController.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/URCombatSet.h"
#include "AbilitySystem/AttributeSet/URHealthSet.h"
#include "AbilitySystem/AttributeSet/URPlayerResourceSet.h"
#include "BlueprintLibrary/URWidgetBlueprintLibrary.h"
#include "Character/URPlayerResourceComponent.h"
#include "GameModes/URGameGameMode.h"
#include "UI/HUD/URGameHUD.h"

AURGamePlayerState::AURGamePlayerState()
{
	URASC = CreateDefaultSubobject<UURAbilitySystemComponent>(TEXT("UR ASC"));
	URASC->SetIsReplicated(true);
	URASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	PlayerResourceComponent = CreateDefaultSubobject<UURPlayerResourceComponent>(TEXT("Player Resource Component"));
	
	HealthSet = CreateDefaultSubobject<UURHealthSet>(TEXT("Health Set"));
	CombatSet = CreateDefaultSubobject<UURCombatSet>(TEXT("Combat Set"));
	PlayerResourceSet = CreateDefaultSubobject<UURPlayerResourceSet>(TEXT("Player Resource Set"));
	
	// AbilitySystemComponent needs to be updated at a high frequency.
	SetNetUpdateFrequency(100.0f);
}

void AURGamePlayerState::ResetForSpawn()
{
	if (PlayerResourceComponent)
	{
		PlayerResourceComponent->SetManaToMax();
	}
}

void AURGamePlayerState::InitAbilitySystemComponent()
{
	IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(PlayerResourceComponent, URASC);
}

void AURGamePlayerState::NotifyPlayerAbilitySystemReady()
{
	if (HasAuthority())
	{
		if (AURGameGameMode* GameGM = GetWorld()->GetAuthGameMode<AURGameGameMode>())
		{
			GameGM->UpdateIsAllPlayersDead();
		}
	}
}

bool AURGamePlayerState::IsDead() const
{
	if (URASC)
	{
		return URASC->IsDead();
	}
	
	return false;
}

void AURGamePlayerState::ClientOnRespawn_Implementation()
{
	UURWidgetBlueprintLibrary::ClosePopup(GetPlayerController(), EPopupType::PlayerDied);
}

void AURGamePlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AController* Controller = GetOwningController())
	{
		if (Controller->GetClass()->ImplementsInterface(UURAbilitySystemComponentReceiverInterface::StaticClass()))
		{
			IURAbilitySystemComponentReceiverInterface::Execute_ReceiveAbilitySystemComponent(Controller, URASC);
		}
	}
}