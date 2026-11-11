// Written by EXPx2


#include "Interface/URAICharacterInterface.h"

// Add default functionality here for any IAIInterface functions that are not pure virtual.
float IURAICharacterInterface::GetPatrolRadius_Implementation() const
{
	return 500.f;
}

bool IURAICharacterInterface::GetIsAggressive_Implementation() const
{
	return false;
}

float IURAICharacterInterface::GetDetectRange_Implementation() const
{
	return 800.f;
}

float IURAICharacterInterface::GetActiveRange_Implementation() const
{
	return 3000.f;
}

class UAIAbilityOrderSet* IURAICharacterInterface::GetAbilityOrderSet_Implementation() const
{
	return nullptr;
}
