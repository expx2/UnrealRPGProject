// Written by EXPx2


#include "URAbilityAction_CameraLensEffect.h"

#include "URAbilityAction_Custom.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "Particles/EmitterCameraLensEffectBase.h"

bool FURAbilityAction_CameraLensEffect::HasValidData() const
{
	return EffectClass.Get() != nullptr;
}

FURAbilityActionStartResult FURAbilityAction_CameraLensEffect::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability,
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	APlayerController* PC = Ability->GetActorInfo().PlayerController.Get();
	if (!PC)
	{
		return FURAbilityActionStartResult::Failed();
	}
	if (!PC->IsLocalPlayerController())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	APlayerCameraManager* CameraManager = PC->PlayerCameraManager.Get();
	if (!CameraManager)
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	CameraManager->AddGenericCameraLensEffect(EffectClass);
	
	return FURAbilityActionStartResult::Completed();
}
