// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityActionTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityActionTaskDelegate, UAbilityActionTask*, Task);

/**
 * 
 */
UCLASS()
class URGAME_API UAbilityActionTask : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	/** 성공적으로 태스크 완료되었을 때 호출 */
	UPROPERTY(BlueprintAssignable)
	FAbilityActionTaskDelegate OnCompleted;

	/** 태스크 시작 혹은 진행 중 실패했을 때 호출 */
	UPROPERTY(BlueprintAssignable)
	FAbilityActionTaskDelegate OnFailed;
	
protected:
	UFUNCTION(BlueprintCallable, Category="UR|Ability")
	virtual bool FinishTask(const bool bIsSuccess);
	
	
	
	bool bIsFinished = false;
};
