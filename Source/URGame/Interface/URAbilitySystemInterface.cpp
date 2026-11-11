// Written by EXPx2

#include "Interface/URAbilitySystemInterface.h"

#include "AbilitySystem/URAbilitySystemComponent.h"

UAbilitySystemComponent* IURAbilitySystemInterface::GetAbilitySystemComponent() const
{
	return GetURAbilitySystemComponent();
}
