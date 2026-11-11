// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_WaitTargetInRange.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitTargetInRangeDelegate);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FWaitTargetInRangeValidateDelegate, AActor*, Actor);

/**
 * 타겟이 일정 범위안에 들어왔는지 체크한다.
 * 일관성을 위해 UPathFollowingComponent의 HasReached를 사용한다.
 * 
 * @see AbilityTask_MoveToActor
 */
UCLASS()
class URGAME_API UAbilityTask_WaitTargetInRange : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	[[deprecated]]
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UAbilityTask_WaitTargetInRange* WaitTargetInRange(UGameplayAbility* OwningAbility, AActor* TargetActor, const float Range);
	
	virtual void TickTask(float DeltaTime) override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
	/** 성공적으로 범위안에 도달하면 호출 */
	UPROPERTY(BlueprintAssignable)
	FWaitTargetInRangeDelegate OnTargetInRange;

	/** 제한 시간 종료나 실패 시 호출 */
	UPROPERTY(BlueprintAssignable)
	FWaitTargetInRangeDelegate OnFailed;
	
	/** 타겟이 파괴되거나 Validate 실패 시 호출 */
	UPROPERTY(BlueprintAssignable)
	FWaitTargetInRangeDelegate OnCancelled;
	
	/** 타겟이 유효한지 검사하여 실패하면 Cancel */
	FWaitTargetInRangeValidateDelegate Validate;
	
	/** 일정 시간 액터가 움직이지 않으면 실패로 처리함 */
	UPROPERTY(BlueprintReadWrite, Category="UR")
	float MaxStoppingDuration = 1.f;
	
protected:
	virtual void Activate() override;
	
	UFUNCTION()
	void HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	
	void FinishTask(const bool bIsReached, const bool bIsCancelled);
	
	TWeakObjectPtr<AActor> TargetActorPtr;
	float Range = 0.0f;
	
	TWeakObjectPtr<class UPathFollowingComponent> PathFollowingComponent;
	
	TWeakObjectPtr<class UCharacterMovementComponent> CharacterMovementComponent;
	
	float StoppingDuration = 0.f;
	
	bool bIsFinished = false;
};
