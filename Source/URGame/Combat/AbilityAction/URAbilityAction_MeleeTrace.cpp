// Written by EXPx2


#include "URAbilityAction_MeleeTrace.h"

#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_MeleeTrace.h"

bool FURAbilityAction_MeleeTrace::HasValidData() const
{
	if (!Shape.IsValid() ||
		MaxTargetCount < 1 || 
		HitCount < 1)
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

FURAbilityActionStartResult FURAbilityAction_MeleeTrace::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	UAbilityActionTask_MeleeTrace* MeleeTrace =
		UAbilityActionTask_MeleeTrace::MeleeTrace(
			Ability,
			*this,
			InContext.TargetActorPtr.Get(),
			GetActionElapsedTime(TimelineElapsed));
	
	return FURAbilityActionStartResult::Running(MeleeTrace);
}
