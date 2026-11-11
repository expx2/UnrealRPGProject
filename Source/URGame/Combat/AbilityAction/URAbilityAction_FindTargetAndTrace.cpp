// Written by EXPx2


#include "URAbilityAction_FindTargetAndTrace.h"

#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_FindTargetAndTrace.h"

bool FURAbilityAction_FindTargetAndTrace::HasValidData() const
{
	if (!Shape.IsValid() ||
		MaxFindingTargetCount < 1 ||
		MaxTargetCount < 1)
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

FURAbilityActionStartResult FURAbilityAction_FindTargetAndTrace::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	UAbilityActionTask_FindTargetAndTrace* TraceAtTarget =
		UAbilityActionTask_FindTargetAndTrace::FindTargetAndTrace(
			Ability,
			*this,
			InContext.TargetActorPtr.Get(),
			GetActionElapsedTime(TimelineElapsed));
	
	return FURAbilityActionStartResult::Running(TraceAtTarget);
}
