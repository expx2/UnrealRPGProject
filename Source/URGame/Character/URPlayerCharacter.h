// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Character/URASCCharacter.h"
#include "URPlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API AURPlayerCharacter : public AURASCCharacter
{
	GENERATED_BODY()
	
public:
	AURPlayerCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	virtual void PossessedBy(AController* NewController) override;
	
	virtual void UnPossessed() override;
	
	virtual void PawnClientRestart() override;
	
	virtual void OnRep_PlayerState() override;

	virtual void ResetForSpawn() override;
	
	// ~Begin IURTargetableInterface
	virtual EURFaction GetFaction_Implementation() const override;
	// ~End IURTargetableInterface
	

protected:
	virtual void BeginPlay() override;
	
	virtual void SetAbilitySystemComponent() override;

	virtual void HandleDeath(AActor* Actor) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURCharacterInputComponent> CharacterInputComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURCameraBoomComponent> CameraBoom;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UCameraComponent> FollowCamera;
	
	FDelegateHandle InCombatStateChangedDelegateHandle;
};
