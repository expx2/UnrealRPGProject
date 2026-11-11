// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueInterface.h"
#include "Character/URCharacter.h"
#include "Interface/URAbilitySystemInterface.h"
#include "Interface/URDamageableInterface.h"
#include "Interface/URTargetableInterface.h"
#include "URASCCharacter.generated.h"

class AURASCCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FURAbilitySystemComponentInitalized, UAbilitySystemComponent*, ASC);

/**
 * 
 */
UCLASS()
class URGAME_API AURASCCharacter : public AURCharacter,
                                   public IURAbilitySystemInterface,
                                   public IURTargetableInterface,
                                   public IURDamageableInterface,
                                   public IGameplayCueInterface
{
	GENERATED_BODY()
	
public:
	AURASCCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnRep_Controller() override;
	
	virtual FRotator GetBaseAimRotation() const override;
	
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;
	
	virtual void ClearAbilitySystemComponent();
	
	UFUNCTION(BlueprintCallable, Category="UR|Character")
	virtual void ResetForSpawn();
	
	// ~Begin IURAbilitySystemInterface
	virtual UURAbilitySystemComponent* GetURAbilitySystemComponent() const override { return URASC; }
	// ~End IURAbilitySystemInterface
	
	// ~Begin IURTargetableInterface
	virtual FName GetDisplayName_Implementation() const override { return CharacterCode; }
	// ~End IURTargetableInterface

protected:
	virtual void BeginPlay() override;
	
	/** AbilitySystemComponent의 ActorInfo를 설정하고 초기화를 호출 */
	virtual void SetAbilitySystemComponent();
	
	virtual void InitAbilitySystemComponent();
	
	UFUNCTION(BlueprintImplementableEvent, Category="UR|Character")
	void K2_OnAbilitySystemInitialized(UAbilitySystemComponent* AbilitySystemComponent);
	
	void SetMovementModeTag(const EMovementMode MovementMode, const bool bTagEnabled) const;
	
	UFUNCTION(BlueprintNativeEvent, Category="UR|Character")
	void GameplayCue_DamageTaken(const EGameplayCueEvent::Type CueEvent, const FGameplayCueParameters& CueParameters);
	
	virtual void GameplayCue_DamageTaken_Implementation(const EGameplayCueEvent::Type CueEvent, const FGameplayCueParameters& CueParameters);
	
	UFUNCTION()
	virtual void HandleDeath(AActor* Actor);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	FName CharacterCode = FName("ASC Character");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURHealthComponent> HealthComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURCombatComponent> CombatComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURActorDisplayWidgetComponent> ActorDisplayWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURDamageTakenTimelineComponent> DamageTakenTimelineComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURDeathTimelineComponent> DeathTimelineComponent;
	
	UPROPERTY(BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURAbilitySystemComponent> URASC;
	
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TObjectPtr<class UURGameplayAbilitySet> AbilitySet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UURAbilityTagRelationshipMapping> TagRelationshipMapping;
	
	UPROPERTY(EditDefaultsOnly, Category="UR")
	TObjectPtr<class UAnimMontage> DeathAnimMontage;
	
	// AbilitySet 등으로 부여한 어빌리티 목록
	TArray<struct FGameplayAbilitySpecHandle> GrantedAbilitySpecHandles;
};
