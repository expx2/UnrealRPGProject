// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Combat/AbilityAction/URAbilityActionBase.h"
#include "StructUtils/InstancedStruct.h"
#include "AbilityTask_AbilityActionTimeline.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAbilityActionTimelineDelegate);

UENUM()
enum class EURAbilityActionState : uint8
{
	// SkillAction이 유효하지 않음
	None, 
	// 대기 중
	Pending, 
	// 진행 중
	Running, 
	// 정상 완료됨
	Completed, 
	// 실패했으나 Continue됨
	Continued, 
};


UENUM()
enum class EURBlockState : uint8
{
	// 설정 전
	NotSet,
	// 설정함
	Set,
	// 완료됨
	Done,
};


/**
 * 
 */
UCLASS()
class URGAME_API UAbilityTask_AbilityActionTimeline : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	static UAbilityTask_AbilityActionTimeline* AbilityActionTimeline(UURGameplayAbility_ActiveAbility* OwningAbility, const TArray<TInstancedStruct<FURAbilityActionBase>>& Actions, const FURAbilityActionContext& ActionContext, const double LatencyCompensation, const FFloatRange& BlockMovementTimeRange, const FFloatRange& BlockRotationTimeRange);
	
	virtual void TickTask(float DeltaTime) override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	/** 모든 액션이 완료되었을 때 호출. 실패 정책이 Continue인 액션이 실패했을 경우에도 호출된다. */
	UPROPERTY(BlueprintAssignable)
	FAbilityActionTimelineDelegate OnCompleted;
	
	/** 데이터 오류가 있는 액션이 있거나, 실패 정책이 AbortTimeline인 액션이 실패했을 경우 호출된다. */
	UPROPERTY(BlueprintAssignable)
	FAbilityActionTimelineDelegate OnFailed;
	
protected:
	virtual void Activate() override;

	UURGameplayAbility_ActiveAbility* GetURAbility() const;

	void UpdateActions(double CurrentSeconds);
	
	void UpdateBlock(double CurrentSeconds);
	
	void BlockMovement();
	
	void UnblockMovement();
	
	void BlockRotation();
	
	void UnblockRotation();
	
	UFUNCTION()
	void HandleTaskCompleted(UAbilityActionTask* Task);
	
	UFUNCTION()
	void HandleTaskFailed(UAbilityActionTask* Task);
	
	void CheckFinish();
	
	void FinishTask(bool bIsSuccess);
	
	
	TArray<TInstancedStruct<FURAbilityActionBase>> Actions;
	FURAbilityActionContext ActionContext;
	double LatencyCompensation;
	FFloatRange BlockMovementTimeRange;
	FFloatRange BlockRotationTimeRange;
	
	bool bIsFinished = false;
	
	double ActivationWorldSeconds;
	
	TArray<EURAbilityActionState> ActionStates;
	
	/** 실행한 비동기 AbilityTask의 Index 맵 */
	TMap<TObjectPtr<UAbilityActionTask>, int32> AsyncTaskMap;
	
	EURBlockState MovementBlockState = EURBlockState::NotSet;
	EURBlockState RotationBlockState = EURBlockState::NotSet;
};
