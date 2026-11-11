// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilityActionTask.h"
#include "Combat/AbilityAction/URAbilityAction_HitTarget.h"
#include "AbilityActionTask_HitTarget.generated.h"

UENUM()
enum class EURHitTargetTaskPhase : uint8
{
	None,
	Action,
	Done,
};

UCLASS()
class URGAME_API UAbilityActionTask_HitTarget : public UAbilityActionTask
{
	GENERATED_BODY()

public:
	static UAbilityActionTask_HitTarget* HitTarget(class UURGameplayAbility_ActiveAbility* OwningAbility, const FURAbilityAction_HitTarget& ActionData, AActor* TargetActor, float ElapsedTime);

	virtual void TickTask(float DeltaTime) override;

protected:
	virtual void Activate() override;

	virtual bool FinishTask(const bool bIsSuccess) override;

	void UpdateTask();

	void DoAction();

	void Hit(AActor* TargetActor);

	void SetPhase(EURHitTargetTaskPhase NewPhase);

	FURAbilityAction_HitTarget ActionData;
	TWeakObjectPtr<AActor> AbilityTargetActor;
	float ElapsedTime;

	double ActivationWorldSeconds;
	EURHitTargetTaskPhase CurrentPhase;
	int ActionCount = 0;
	
	double NextActionTime;
};
