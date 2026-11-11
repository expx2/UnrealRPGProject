// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"
#include "Combat/URHitEffect.h"
#include "StructUtils/InstancedStruct.h"

#include "URAbilityAction_FindTargetAndTrace.generated.h"


USTRUCT(BlueprintType, meta=(DisplayName="Find Target And Trace"))
struct FURAbilityAction_FindTargetAndTrace : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override;
	
	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
	
	/** Trace를 진행할 타겟의 최대 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	int MaxFindingTargetCount = 1; 
	
	/** Trace를 진행할 타겟을 찾는 최대 범위 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	float FindingTargetRange = 1000.f;
	
	/** 캐스팅 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Casting")
	bool bHasCasting = false;

	/** 캐스팅 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Casting", meta=(EditCondition="bHasCasting == true", EditConditionHides, Units="s"))
	float CastingDuration = 0.0f;

	/** 캐스팅 중 적 추적 시간. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Casting", meta=(EditCondition="bHasCasting == true", EditConditionHides, Units="s"))
	float ChasingDurationDuringCasting = 0.0f;

	/** 인디케이터 생성 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR|Casting", meta=(EditCondition="bHasCasting == true", EditConditionHides))
	bool bShowIndicator = false;
	
	/** 범위 정보 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(ExcludeBaseStruct))
	TInstancedStruct<struct FURColliderShape> Shape;
	
	/** 최대 타격 가능한 대상 수 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	int MaxTargetCount = 1; 
	
	/** 전달할 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TArray<FURHitEffect> HitEffects;
	
	/** Hit 위치에 생성할 파티클 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<UParticleSystem> HitParticle;
	
	/** 피격 GC 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Categories="GameplayCue.HitImpact"))
	FGameplayTag HitImpactCueTag;
};
