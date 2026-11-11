// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "URAbilityActionBase.h"
#include "Character/URTargetingComponent.h"

#include "URAbilityAction_SpawnHitTraceActor.generated.h"

USTRUCT(BlueprintType, meta=(DisplayName="Spawn Hit Trace Actor"))
struct FURAbilityAction_SpawnHitTraceActor : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override;

	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TSubclassOf<class AHitTraceActor> HitTraceActorClass;
	
	/** 참이면 소스 액터의 메시 소켓에 부착, 거짓이면 Context의 Location에 생성 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	bool bAttach = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(EditCondition="bAttach == true", EditConditionHides))
	FName SocketName = NAME_None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	FVector LocationOffset = FVector::ZeroVector;
	
	/** 전달할 GE */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TArray<struct FURHitEffect> HitEffects;
	
	/** 피격 태그 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Categories="GameplayCue.HitImpact"))
	FGameplayTag HitImpactTag;
};
