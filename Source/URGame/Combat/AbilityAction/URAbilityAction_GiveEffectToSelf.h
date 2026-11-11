// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"

#include "URAbilityAction_GiveEffectToSelf.generated.h"

USTRUCT(BlueprintType, meta=(DisplayName="Give Effect To Self"))
struct FURAbilityAction_GiveEffectToSelf : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override;
	
	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
	
protected:
	/** 부여할 이펙트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TArray<struct FURHitEffect> HitEffects;
};