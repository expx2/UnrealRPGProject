// Written by EXPx2


#include "AbilitySystem/GameplayEffect/GameplayEffect_ManaCost.h"
#include "AbilitySystem/AttributeSet/URPlayerResourceSet.h"
#include "GameplayTags/URGameplayTags.h"

UGameplayEffect_ManaCost::UGameplayEffect_ManaCost()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat ManaCostMagnitude;
	ManaCostMagnitude.DataTag = URGameplayTags::SetByCaller_ManaCost;

	FGameplayModifierInfo& ManaModifier = Modifiers.AddDefaulted_GetRef();
	ManaModifier.Attribute = UURPlayerResourceSet::GetManaAttribute();
	ManaModifier.ModifierOp = EGameplayModOp::Additive;
	ManaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(ManaCostMagnitude);
}
