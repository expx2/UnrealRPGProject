// Written by EXPx2


#include "Interface/URDamageableInterface.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Definitions/URFaction.h"
#include "GameplayTags/URGameplayTags.h"

// Add default functionality here for any IURDamageableInterface functions that are not pure virtual.

EURFaction IURDamageableInterface::GetFaction_Implementation() const
{
	return EURFaction::Neutral;
}

void IURDamageableInterface::NativeTakeDamage_Implementation(const float Damage, const bool bIsCritical)
{
}

bool IURDamageableInterface::CanTakeDamage_Implementation() const
{
	if (AActor* Actor = Cast<AActor>(_getUObject()))
	{
		if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
		{
			return !ASC->HasMatchingGameplayTag(URGameplayTags::Character_State_CantTakeDamage);
		}
	}

	return true;
}

