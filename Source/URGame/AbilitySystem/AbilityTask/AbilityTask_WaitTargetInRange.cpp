// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityTask_WaitTargetInRange.h"

#include "AIController.h"
#include "URGame.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"

UAbilityTask_WaitTargetInRange* UAbilityTask_WaitTargetInRange::WaitTargetInRange(
	UGameplayAbility* OwningAbility,
	AActor* TargetActor, 
	const float Range)
{
	UAbilityTask_WaitTargetInRange* Task = NewAbilityTask<UAbilityTask_WaitTargetInRange>(OwningAbility);
	Task->TargetActorPtr = TargetActor;
	// 정지 거리에 타겟의 반경을 포함
	// if (const INavAgentInterface* GoalNavAgent = Cast<const INavAgentInterface>(InTargetActor))
	// {
	// 	const float TargetAgentRadius = GoalNavAgent->GetNavAgentPropertiesRef().AgentRadius;
	// 	if (FMath::IsFinite(TargetAgentRadius) && TargetAgentRadius > 0.0f)
	// 	{
	// 		Task->Range = FMath::Max(InRange, 0.f) + TargetAgentRadius;
	// 		return Task;
	// 	}
	// }
	float ActorRadius = TargetActor->GetSimpleCollisionRadius();
	Task->Range = FMath::Max(Range, 0.f) + ActorRadius;
	return Task;
}

void UAbilityTask_WaitTargetInRange::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	AActor* Target = TargetActorPtr.Get();
	if (!Target ||
		!PathFollowingComponent.IsValid() ||
		!CharacterMovementComponent.IsValid())
	{
		FinishTask(false, false);
		return;
	}
	
	if (Validate.IsBound() && !Validate.Execute(Target))
	{
		FinishTask(false, true);
		return;
	}

	if (const bool bIsReached = PathFollowingComponent->HasReached(*Target, EPathFollowingReachMode::ExactLocation, Range))
	{
		// 도달함
		FinishTask(true, false);
		return;
	}
	
	if (CharacterMovementComponent.Get()->GetCurrentAcceleration().SizeSquared() < 0.1f)
	{
		// 오랜 시간 가만히 멈춰있으면 종료
		StoppingDuration += DeltaTime;
		if (StoppingDuration >= MaxStoppingDuration)
		{
			FinishTask(false, false);
			return;
		}
	}
	else
	{
		StoppingDuration = 0.f;
	}
}

void UAbilityTask_WaitTargetInRange::OnDestroy(bool bInOwnerFinished)
{
	if (AActor* Actor = TargetActorPtr.Get())
	{
		Actor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleTargetEndPlay);
	}
	TargetActorPtr.Reset();
	
	PathFollowingComponent.Reset();
	
	StoppingDuration = 0.f;
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_WaitTargetInRange::Activate()
{
	Super::Activate();
	
	StoppingDuration = 0.f;
	
	AActor* Target = TargetActorPtr.Get();
	if (!Target)
	{
		FinishTask(false, true);
		return;
	}
	
	if (Validate.IsBound() && !Validate.Execute(Target))
	{
		FinishTask(false, true);
		return;
	}
	
	const APawn* AvatarPawn = Cast<APawn>(GetAvatarActor());
	if (!IsValid(AvatarPawn))
	{
		FinishTask(false, false);
		return;
	}
	
	CharacterMovementComponent = Cast<UCharacterMovementComponent>(AvatarPawn->GetMovementComponent());
	if (!CharacterMovementComponent.IsValid())
	{
		FinishTask(false, false);
		return;
	}
	
	AController* Controller = AvatarPawn->GetController();
	if (!IsValid(Controller))
	{
		FinishTask(false, false);
		return;
	}
	
	// AIBlueprintHelperLibrary::SimpleMoveToActor 참고함
	UPathFollowingComponent* PFollowComp = UURGameBlueprintLibrary::GetPathFollowingComponent(Controller, true);
	
	if (!IsValid(PFollowComp) || !PFollowComp->IsPathFollowingAllowed())
	{
		FinishTask(false, false);
		return;
	}
	
	PathFollowingComponent = PFollowComp;
	
	if (const bool bIsReached = PFollowComp->HasReached(*Target, EPathFollowingReachMode::ExactLocation, Range))
	{
		// 시작 순간 범위 안에 있을 경우
		FinishTask(true, false);
		return;
	}
	
	bTickingTask = true;
	Target->OnEndPlay.AddUniqueDynamic(this, &ThisClass::HandleTargetEndPlay);
}

void UAbilityTask_WaitTargetInRange::HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (Actor == TargetActorPtr.Get())
	{
		FinishTask(false, true);
	}
}

void UAbilityTask_WaitTargetInRange::FinishTask(const bool bIsReached, const bool bIsCancelled)
{
	if (bIsFinished)
	{
		// 중복 호출 방지
		return;
	}

	bIsFinished = true;
	bTickingTask = false;
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bIsReached)
		{
			OnTargetInRange.Broadcast();
		}
		else if (bIsCancelled)
		{
			OnCancelled.Broadcast();
		}
		else
		{
			OnFailed.Broadcast();
		}
	}

	EndTask();
}
