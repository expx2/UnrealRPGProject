// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "URAbilityActionBase.h"

#include "URAbilityAction_MoveTo.generated.h"

UENUM()
enum class EMoveToType : uint8
{
	Offset, // 사용자 기준 오프셋
	ToTargetActor, // 타겟 액터 위치로 이동
	ToLocation, // 타겟 위치로 이동
};

USTRUCT(BlueprintType, meta=(DisplayName="Move To"))
struct FURAbilityAction_MoveTo : public FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual bool HasValidData() const override { return Duration > SMALL_NUMBER; }

	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	EMoveToType MoveToType = EMoveToType::ToLocation;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(EditCondition="MoveToType == EMoveToType::Offset", EditConditionHides))
	FVector Offset = FVector::ZeroVector;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	bool bLookGoal = true;
	
	/** 이동 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Units="s", ClampMin=0, UIMin=0))
	float Duration = 1.0f;

	/** 타겟과의 거리 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Units="cm", ClampMin=0, UIMin=0))
	float Distance = 0.0f;

	/** 이동 경로에 적용할 오프셋 커브. 비어 있으면 직선으로 이동한다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TObjectPtr<class UCurveVector> VectorCurve;
};