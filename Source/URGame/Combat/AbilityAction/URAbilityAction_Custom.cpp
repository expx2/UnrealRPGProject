// Written by EXPx2


#include "URAbilityAction_Custom.h"

#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_Custom.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"

bool FURAbilityAction_Custom::HasValidData() const
{
	return true;
}

FURAbilityActionStartResult FURAbilityAction_Custom::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	UAbilityActionTask_Custom* Task = UAbilityActionTask_Custom::CustomAction(Ability, InContext, FMath::Max(TimelineElapsed - StartTime, 0.f));
	
	return FURAbilityActionStartResult::Running(Task);
}
