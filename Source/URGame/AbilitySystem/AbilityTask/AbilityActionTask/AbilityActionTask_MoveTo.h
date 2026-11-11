// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask.h"
#include "AbilityActionTask_MoveTo.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UAbilityActionTask_MoveTo : public UAbilityActionTask
{
	GENERATED_BODY()
	
public:
	static UAbilityActionTask_MoveTo* MoveToActor(class UURGameplayAbility_ActiveAbility* OwningAbility, const FVector& TargetLocation, float Duration, class UCurveVector* VectorCurve, float ElapsedTime);
	
	virtual void TickTask(float DeltaTime) override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
protected:
	
	virtual void Activate() override;
	
	virtual bool FinishTask(const bool bIsSuccess) override;
	
	FVector TargetLocation;
	float Duration;
	TWeakObjectPtr<UCurveVector> VectorCurve;
	float ElapsedTime;
	
	double EndTime;
	
	TWeakObjectPtr<class UCharacterMovementComponent> MovementComponent;
	
	uint16 RootMotionSourceID;
};
