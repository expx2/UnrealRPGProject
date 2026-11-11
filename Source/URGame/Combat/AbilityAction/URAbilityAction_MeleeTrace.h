// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"
#include "Combat/URHitEffect.h"
#include "StructUtils/InstancedStruct.h"

#include "URAbilityAction_MeleeTrace.generated.h"

USTRUCT(BlueprintType, meta=(DisplayName="Melee Trace"))
struct FURAbilityAction_MeleeTrace : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override;
	
	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;

	/** 캐스팅 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Casting")
	bool bHasCasting = false;

	/** 캐스팅 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Casting", meta=(EditCondition="bHasCasting == true", EditConditionHides, Units="s"))
	float CastingDuration = 0.0f;

	/** 인디케이터 생성 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Casting", meta=(EditCondition="bHasCasting == true", EditConditionHides))
	bool bShowIndicator = false;
	
	/** 범위 정보 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ExcludeBaseStruct))
	TInstancedStruct<struct FURColliderShape> Shape;
	
	/** 기준 지점 오프셋 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	FVector LocationOffset = FVector::ZeroVector;
	
	/** 기준 방향 오프셋 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(UIMin=-180, UIMax=180, ClampMin=-180, ClampMax=180, Units="deg"))
	float DirectionOffset = 0.f;
	
	/** 최대 타격 가능한 대상 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	int MaxTargetCount = 1; 
	
	/** 타격 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ClampMin=1, UIMin=1))
	int HitCount = 1;

	/** 타격 인터벌 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(EditCondition="HitCount > 1", EditConditionHides, Units="s", ClampMin=0.02, UIMin=0.02))
	float HitInterval = 0.1f;
	
	/** 전달할 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TArray<FURHitEffect> HitEffects;
	
	/** 피격 GC 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Categories="GameplayCue.HitImpact"))
	FGameplayTag HitImpactCueTag;
	
	
};
