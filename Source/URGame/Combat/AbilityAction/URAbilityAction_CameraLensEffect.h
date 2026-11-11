// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"

#include "URAbilityAction_CameraLensEffect.generated.h"

USTRUCT(BlueprintType, meta=(DisplayName="Camera Lens Effect"))
struct FURAbilityAction_CameraLensEffect : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override;
	
	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TSubclassOf<class AEmitterCameraLensEffectBase> EffectClass;
};