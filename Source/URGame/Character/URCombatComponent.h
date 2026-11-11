// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "Interface/URAbilitySystemInterface.h"
#include "URCombatComponent.generated.h"


class UURAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class URGAME_API UURCombatComponent : public UActorComponent,
                                      public IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()

public:
	UURCombatComponent();

	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void ReceiveAbilitySystemComponent_Implementation(class UAbilitySystemComponent* AbilitySystemComponent) override;
	
	virtual void ClearAbilitySystemComponent_Implementation() override;
	
	void SetCombatTargetActor(AActor* Actor);
	
	bool GetIsInCombat() const;
	
	void Clear();
	
	AActor* GetCombatTargetActor() const { return CombatTargetActor.Get(); }

protected:
	UFUNCTION()
	void OnRep_CombatTarget();

	void UpdateRotation() const;
	
	void HandleInCombatTagChanged(FGameplayTag GameplayTag, int Count);
	
	UFUNCTION()
	virtual void HandleDamageTaken(AActor* Instigator, AActor* Causer, float Magnitude);
	
	UPROPERTY()
	TWeakObjectPtr<class UAbilitySystemComponent> AbilityComponent;
	
	UPROPERTY()
	TWeakObjectPtr<class UURHealthComponent> HealthComponent;
	
	UPROPERTY()
	TWeakObjectPtr<class UURCharacterMovementComponent> CharacterMovement;
	
	UPROPERTY(ReplicatedUsing = OnRep_CombatTarget)
	TWeakObjectPtr<AActor> CombatTargetActor;
	
	
	FDelegateHandle InCombatTagChangedDelegateHandle;
};
