// Written by EXPx2

#pragma once

#include "CoreMinimal.h"

#include "URAbilityActionBase.generated.h"

/** AbilityAction의 태스크가 실패했을 때 정책 */
UENUM(BlueprintType)
namespace EAbilityActionFailedPolicy
{
	enum Type : int
	{
		// 타임라인 즉시 종료
		AbortTimeline, 
		// 타임라인 지속
		Continue, 
	};
}

/** AbilityAction의 태스크 시작 시 상태 */
UENUM(BlueprintType)
enum class EURAbilityActionStartState : uint8
{
	Completed,
	Running,
	Failed,
};

/** 시작 후 즉시 종료 되었는지, 실패했는지, 비동기 Task가 실행되었다면 Task 전달 */
USTRUCT(BlueprintType)
struct FURAbilityActionStartResult
{
	GENERATED_BODY()
	
public:
	static FURAbilityActionStartResult Completed()
	{
		return { EURAbilityActionStartState::Completed, nullptr };
	}

	static FURAbilityActionStartResult Running(class UAbilityActionTask* InTask)
	{
		return { EURAbilityActionStartState::Running, InTask };
	}

	static FURAbilityActionStartResult Failed()
	{
		return { EURAbilityActionStartState::Failed, nullptr };
	}

	EURAbilityActionStartState State = EURAbilityActionStartState::Failed;

	UAbilityActionTask* Task = nullptr;
};

USTRUCT(BlueprintType)
struct FURAbilityActionContext
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FVector Location = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> TargetActorPtr;
};

USTRUCT(BlueprintType)
struct FURAbilityActionBase
{
	GENERATED_BODY()
	
public:
	virtual ~FURAbilityActionBase() = default;
	virtual bool HasValidData() const { return true; }
	
	virtual FURAbilityActionStartResult TryStartAbilityAction(class UURGameplayAbility_ActiveAbility* Ability, FURAbilityActionContext& InContext, const float TimelineElapsed) const { return FURAbilityActionStartResult::Completed(); }
	
	/** AbilityAction의 태스크가 실패했을 때 정책 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR")
	TEnumAsByte<EAbilityActionFailedPolicy::Type> FailedPolicy = EAbilityActionFailedPolicy::Continue;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UR", meta=(Units="s", DisplayPriority=-10, ClampMin=0, UIMin=0))
	float StartTime = 0.f;
	
protected:
	virtual float GetActionElapsedTime(const float TimelineElapsed) const
	{
		return FMath::Max(TimelineElapsed - FMath::Max(StartTime, 0.0f), 0.0f);
	}
};