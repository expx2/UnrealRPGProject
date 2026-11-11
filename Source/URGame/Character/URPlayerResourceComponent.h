// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSet/URAttributeSet.h"
#include "Components/ActorComponent.h"
#include "Interface/URAbilitySystemComponentReceiverInterface.h"
#include "URPlayerResourceComponent.generated.h"

class UURAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FURPlayerResource_AttributeChangedEvent, UURPlayerResourceComponent*, PlayerResourceComponent, float, OldValue, float, NewValue);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class URGAME_API UURPlayerResourceComponent : public UActorComponent,
                                              public IURAbilitySystemComponentReceiverInterface
{
	GENERATED_BODY()

public:
	UURPlayerResourceComponent(const FObjectInitializer& ObjectInitializer);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void ReceiveAbilitySystemComponent_Implementation(class UAbilitySystemComponent* AbilitySystemComponent) override;
	
	virtual void ClearAbilitySystemComponent_Implementation() override;
	
	UFUNCTION(BlueprintCallable, Category="UR|PlayerResource")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category="UR|PlayerResource")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category="UR|PlayerResource")
	float GetPercent() const
	{
		const float MaxMana = GetMaxMana();
		const float Mana = GetMana();
		if (Mana <= SMALL_NUMBER ||
			MaxMana <= SMALL_NUMBER)
		{
			return 0.0f;
		}
		return FMath::Clamp(Mana / MaxMana, 0.0f, 1.0f);
	}

	void SetManaToMax();

	// Delegate fired when the max mana value has changed. This is called on the client.
	UPROPERTY(BlueprintAssignable)
	FURPlayerResource_AttributeChangedEvent OnMaxManaChanged;

	// Delegate fired when the mana value has changed. This is called on the client.
	UPROPERTY(BlueprintAssignable)
	FURPlayerResource_AttributeChangedEvent OnManaChanged;

protected:
	void HandleMaxManaChanged(float EffectMagnitude, float OldValue, float NewValue);
	
	void HandleManaChanged(float EffectMagnitude, float OldValue, float NewValue);

	void UpdateManaRegen();

	UPROPERTY(EditDefaultsOnly, Category="UR|PlayerResource")
	TSubclassOf<UGameplayEffect> ManaRegenEffect;
	
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> AbilityComponent;

	UPROPERTY()
	TWeakObjectPtr<const class UURPlayerResourceSet> PlayerResourceSet;
	
	FDelegateHandle MaxManaChangedDelegateHandle;
	
	FDelegateHandle ManaChangedDelegateHandle;

	FActiveGameplayEffectHandle ManaRegenSpecHandle;
};
