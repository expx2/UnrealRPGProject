// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"
#include "Combat/URHitEffect.h"

#include "URAbilityAction_HitTarget.generated.h"

USTRUCT(BlueprintType, meta=(DisplayName="Hit Target"))
struct FURAbilityAction_HitTarget : public FURAbilityActionBase
{
	GENERATED_BODY()

public:
	virtual bool HasValidData() const override;

	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
	
	/** 타격 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ClampMin=1, UIMin=1))
	int HitCount = 1;

	/** 타격 인터벌 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(EditCondition="HitCount > 1", EditConditionHides, Units="s", ClampMin=0.02, UIMin=0.02))
	float HitInterval = 0.1f;
	
	/** 전달할 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TArray<FURHitEffect> HitEffects;

	/** 피격 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Categories="GameplayCue.HitImpact"))
	FGameplayTag HitImpactTag;
};
