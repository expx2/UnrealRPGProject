// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSet/URAttributeSet.h"
#include "Components/ActorComponent.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "Interface/URAbilitySystemInterface.h"
#include "URHealthComponent.generated.h"

class UURAbilitySystemComponent;
struct FGameplayEffectSpec;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FURHealth_AttributeChangedEvent, UURHealthComponent*, HealthComponent, float, OldValue, float, NewValue, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FURHealth_DeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FURHealth_DamageTakenEvent, AActor*, EffectInstigator, AActor*, EffectCauser, float, Magnitude);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class URGAME_API UURHealthComponent : public UActorComponent,
                                      public IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UURHealthComponent(const FObjectInitializer& ObjectInitializer);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void ReceiveAbilitySystemComponent_Implementation(class UAbilitySystemComponent* AbilitySystemComponent) override;
	
	virtual void ClearAbilitySystemComponent_Implementation() override;
	
	UFUNCTION(BlueprintCallable, Category="UR|Health")
	float GetMaxHealth() const;
	
	UFUNCTION(BlueprintCallable, Category="UR|Health")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category="UR|Health")
	float GetPercent() const
	{
		const float MaxHealth = GetMaxHealth();
		const float Health = GetHealth();
		if (Health <= SMALL_NUMBER ||
			MaxHealth <= SMALL_NUMBER)
		{
			return 0.0f;
		}
		return FMath::Clamp(Health / MaxHealth, 0.0f, 1.0f);
	}
	
	UFUNCTION(BlueprintCallable, Category="UR|Health")
	void SetHealthToMax();
	
	// Delegate fired when the max health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FURHealth_AttributeChangedEvent OnMaxHealthChanged;
	
	// Delegate fired when the health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FURHealth_AttributeChangedEvent OnHealthChanged;

	// Delegate fired when the character is dead.
	UPROPERTY(BlueprintAssignable)
	FURHealth_DeathEvent OnDeath;
	
	// Delegate fired when the character takes damage.
	UPROPERTY(BlueprintAssignable)
	FURHealth_DamageTakenEvent OnDamageTaken;

protected:
	void HandleDamageTaken(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude);
	
	void HandleMaxHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
	
	void HandleHealthChanged(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
	
	void HandleOutOfHealth(AActor* EffectInstigator, AActor* EffectCauser, const FGameplayEffectSpec* EffectSpec, float EffectMagnitude, float OldValue, float NewValue);
	
	void UpdateHealthRegen(); 
	
	UPROPERTY(EditDefaultsOnly, Category="UR|Health")
	TSubclassOf<UGameplayEffect> HealthRegenEffect;
	
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilityComponent;
	
	UPROPERTY()
	TWeakObjectPtr<const class UURHealthSet> HealthSet;
	
	FDelegateHandle DamageTakenDelegateHandle;
	
	FDelegateHandle MaxHealthChangedDelegateHandle;
	
	FDelegateHandle HealthChangedDelegateHandle;
	
	FDelegateHandle OutOfHealthDelegateHandle;
	
	FActiveGameplayEffectHandle HealthRegenSpecHandle;
};
