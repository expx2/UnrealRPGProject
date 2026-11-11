// Written by EXPx2


#include "AbilitySystem/AttributeSet/URAttributeSet.h"

#include "AbilitySystem/URAbilitySystemComponent.h"

UURAbilitySystemComponent* UURAttributeSet::GetOwningURAbilitySystemComponent() const
{
	return Cast<UURAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
