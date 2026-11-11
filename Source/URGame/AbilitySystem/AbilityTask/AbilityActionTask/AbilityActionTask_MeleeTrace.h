// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask.h"
#include "Combat/AbilityAction/URAbilityAction_MeleeTrace.h"
#include "AbilityActionTask_MeleeTrace.generated.h"

UENUM()
enum class EURMeleeTraceTaskPhase : uint8
{
	None,
	Casting,
	Action,
	Done,
};

/**
 * 
 */
UCLASS()
class URGAME_API UAbilityActionTask_MeleeTrace : public UAbilityActionTask
{
	GENERATED_BODY()
	
public:
	static UAbilityActionTask_MeleeTrace* MeleeTrace(UURGameplayAbility_ActiveAbility* OwningAbility, const FURAbilityAction_MeleeTrace& ActionData, AActor* TargetActor, float ElapsedTime);
	
	virtual void TickTask(float DeltaTime) override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
protected:
	virtual void Activate() override;
	
	virtual bool FinishTask(const bool bIsSuccess) override;
	
	void UpdateTask();
	
	void Trace();
	
	void Hit(TArray<AActor*> Targets);
	
	void SetPhase(EURMeleeTraceTaskPhase NewPhase);
	
	void GetLocationAndDirection(FVector& Location, FVector& Direction) const;
	
	FURAbilityAction_MeleeTrace ActionData;
	
	TWeakObjectPtr<AActor> AbilityTargetActor;
	TWeakObjectPtr<class AAbilityIndicatorActor> Indicator;
	float ElapsedTime;
	
	EURMeleeTraceTaskPhase CurrentPhase;
	double ActivationWorldSeconds;
	double CastingEndTime;
	double NextActionTime;
	int TraceCount = 0;
	
	// 다수의 타격 횟수를 가질 때, 이전 타격에서의 대상 액터들을 저장해 다음 타격에 우선권을 부여함
	TArray<TWeakObjectPtr<AActor>> LastHitActors;
};
