// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystem/URAbilitySystemComponent.h"
#include "URAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_MULTICAST_DELEGATE_ThreeParams(FURAttributeSimpleEvent,
									 float /*EffectMagnitude*/,
									 float /*OldValue*/,
									 float /*NewValue*/);

DECLARE_MULTICAST_DELEGATE_SixParams(FURAttributeEvent,
                                     AActor* /*EffectInstigator*/,
                                     AActor* /*EffectCauser*/,
                                     const FGameplayEffectSpec* /*EffectSpec*/,
                                     float /*EffectMagnitude*/,
                                     float /*OldValue*/,
                                     float /*NewValue*/);

/**
 *
 */
UCLASS()
class URGAME_API UURAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UURAbilitySystemComponent* GetOwningURAbilitySystemComponent() const;
};
