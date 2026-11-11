// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Definitions/URFaction.h"
#include "GameFramework/PlayerState.h"
#include "Interface/URAbilitySystemInterface.h"
#include "Interface/URDamageableInterface.h"
#include "URGame.h"
#include "Definitions/URDefaults.h"
#include "URGamePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API AURGamePlayerState : public APlayerState,
                                      public IURAbilitySystemInterface,
                                      public IURDamageableInterface
{
	GENERATED_BODY()
	
public:
	AURGamePlayerState();
	
	// ~Begin IURAbilitySystemInterface
	virtual UURAbilitySystemComponent* GetURAbilitySystemComponent() const override { return URASC; }
	// ~End IURAbilitySystemInterface
	
	// ~Begin IURDamageableInterface
	virtual EURFaction GetFaction_Implementation() const override { return EURFaction::Ally; }
	// ~End IURDamageableInterface
	
	void ResetForSpawn();
	
	void InitAbilitySystemComponent();
	
	void NotifyPlayerAbilitySystemReady();
	
	bool IsDead() const;
	
	UFUNCTION(Client, Reliable)
	void ClientOnRespawn();
	
	UPROPERTY(BlueprintReadOnly, Category="UR|Game")
	FName PlayingCharacterCode = FURDefaults::DefaultCharacterCode;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<UURAbilitySystemComponent> URASC;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURPlayerResourceComponent> PlayerResourceComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURHealthSet> HealthSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURCombatSet> CombatSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURPlayerResourceSet> PlayerResourceSet;
};
