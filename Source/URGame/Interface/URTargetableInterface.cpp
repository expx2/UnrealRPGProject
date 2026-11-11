// Written by EXPx2


#include "Interface/URTargetableInterface.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Definitions/URFaction.h"
#include "GameplayTags/URGameplayTags.h"

// Add default functionality here for any IURTargetableInterface functions that are not pure virtual.

EURFaction IURTargetableInterface::GetFaction_Implementation() const
{
	return EURFaction::Neutral;
}

bool IURTargetableInterface::CanBeTarget_Implementation() const
{
	if (AActor* Actor = Cast<AActor>(_getUObject()))
	{
		if (Actor->IsHidden())
		{
			return false;
		}
		
		if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
		{
			return !ASC->HasMatchingGameplayTag(URGameplayTags::Character_State_Untargetable);
		}
	}

	return true;
}
