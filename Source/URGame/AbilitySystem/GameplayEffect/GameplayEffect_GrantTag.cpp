// Written by EXPx2


#include "AbilitySystem/GameplayEffect/GameplayEffect_GrantTag.h"

#include "GameplayTags/URGameplayTags.h"

UGameplayEffect_GrantTag::UGameplayEffect_GrantTag()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	FSetByCallerFloat ChainAbilityDuration;
	ChainAbilityDuration.DataTag = URGameplayTags::SetByCaller_GrantTag;
	DurationMagnitude = FGameplayEffectModifierMagnitude(ChainAbilityDuration);
}
