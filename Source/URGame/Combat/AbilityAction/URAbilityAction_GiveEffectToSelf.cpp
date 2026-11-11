// Written by EXPx2

#include "URAbilityAction_GiveEffectToSelf.h"

#include "URGame.h"
#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_Custom.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "BlueprintLibrary/URCombatBlueprintLibrary.h"

bool FURAbilityAction_GiveEffectToSelf::HasValidData() const
{
	return !HitEffects.IsEmpty();
}

FURAbilityActionStartResult FURAbilityAction_GiveEffectToSelf::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	if (!Ability->K2_HasAuthority())
	{
		return FURAbilityActionStartResult::Completed();
	}
	
	UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	const UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability);
	const FGameplayEffectContextHandle EffectContext = ActiveAbility
														   ? ActiveAbility->MakeHitEffectContext(Ability->GetCurrentAbilitySpecHandle(),Ability->GetCurrentActorInfo())
														   : Ability->MakeEffectContext(Ability->GetCurrentAbilitySpecHandle(),Ability->GetCurrentActorInfo());
	
	const bool bIsSuccess = UURCombatBlueprintLibrary::Hit(
		ASC,
		Ability->GetOwningActorFromActorInfo(), 
		HitEffects, 
		EffectContext);
	
	return bIsSuccess ? FURAbilityActionStartResult::Completed() : FURAbilityActionStartResult::Failed();
}
