// Written by EXPx2


#include "URAbilityAction_TraceAtLocation.h"

#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_TraceAtLocation.h"

bool FURAbilityAction_TraceAtLocation::HasValidData() const
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

FURAbilityActionStartResult FURAbilityAction_TraceAtLocation::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	UAbilityActionTask_TraceAtLocation* TraceAtLocation =
		UAbilityActionTask_TraceAtLocation::TraceAtLocation(
			Ability,
			*this,
			InContext.TargetActorPtr.Get(),
			InContext.Location,
			GetActionElapsedTime(TimelineElapsed));
	
	return FURAbilityActionStartResult::Running(TraceAtLocation);
}
