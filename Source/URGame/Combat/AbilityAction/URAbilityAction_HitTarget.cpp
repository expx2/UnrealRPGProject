// Written by EXPx2


#include "URAbilityAction_HitTarget.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_HitTarget.h"

bool FURAbilityAction_HitTarget::HasValidData() const
{
	if (HitCount < 1)
	{
		return false;
	}

	// 부가효과가 비어있는 경우는 PASS
	for (const FURHitEffect& Effect : HitEffects)
	{
		if (!Effect.EffectClass)
		{
			return false;
		}
	}

	return true;
}

FURAbilityActionStartResult FURAbilityAction_HitTarget::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext,
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}

	UAbilityActionTask_HitTarget* HitTarget =
		UAbilityActionTask_HitTarget::HitTarget(
			Ability,
			*this,
			InContext.TargetActorPtr.Get(),
			GetActionElapsedTime(TimelineElapsed));

	return FURAbilityActionStartResult::Running(HitTarget);
}
