// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask.h"
#include "Combat/AbilityAction/URAbilityAction_FindTargetAndTrace.h"
#include "AbilityActionTask_FindTargetAndTrace.generated.h"

UENUM()
enum class EURTraceAtTargetTaskPhase : uint8
{
	None,
	FindingTarget,
	Casting,
	Action,
	Done,
};

USTRUCT()
struct FURTraceAtTargetTargetContainer
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TWeakObjectPtr<AActor> Target;
	
	UPROPERTY()
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY()
	TWeakObjectPtr<class AAbilityIndicatorActor> Indicator;
};

/**
 * 
 */
UCLASS()
class URGAME_API UAbilityActionTask_FindTargetAndTrace : public UAbilityActionTask
{
	GENERATED_BODY()
	
public:
	static UAbilityActionTask_FindTargetAndTrace* FindTargetAndTrace(UURGameplayAbility_ActiveAbility* OwningAbility, const FURAbilityAction_FindTargetAndTrace& ActionData, AActor* TargetActor, float ElapsedTime);
	
	virtual void TickTask(float DeltaTime) override;
	
	virtual void OnDestroy(bool bInOwnerFinished) override;
	
protected:
	virtual void Activate() override;
	
	virtual bool FinishTask(const bool bIsSuccess) override;
	
	void FindTarget();
	
	void UpdateTask();
	
	void ClearInvalidTargetContainer();
	
	void Trace();
	
	void Hit(TArray<AActor*> Targets);
	
	void SetPhase(EURTraceAtTargetTaskPhase NewPhase);
	
	void GetLocationAndDirection(FVector& Location, FVector& Direction) const;
	
	FURAbilityAction_FindTargetAndTrace ActionData;
	TWeakObjectPtr<AActor> AbilityTargetActor;
	float ElapsedTime;
	
	TArray<FURTraceAtTargetTargetContainer> TargetContainers;
	
	double ActivationWorldSeconds;
	EURTraceAtTargetTaskPhase CurrentPhase;
	int ActionCount = 0;
	
	double CastingEndTime;
	double TargetChasingEndTime;
	double NextActionTime;

};
