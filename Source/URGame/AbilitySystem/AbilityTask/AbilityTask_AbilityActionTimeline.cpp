// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityTask_AbilityActionTimeline.h"

#include "AbilitySystemComponent.h"
#include "URGame.h"
#include "AbilityActionTask/AbilityActionTask.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "Combat/AbilityAction/URAbilityActionBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTags/URGameplayTags.h"
#include "StructUtils/InstancedStruct.h"

UAbilityTask_AbilityActionTimeline* UAbilityTask_AbilityActionTimeline::AbilityActionTimeline(
	UURGameplayAbility_ActiveAbility* OwningAbility,
	const TArray<TInstancedStruct<FURAbilityActionBase>>& Actions,
	const FURAbilityActionContext& ActionContext,
	const double LatencyCompensation,
	const FFloatRange& BlockMovementTimeRange,
	const FFloatRange& BlockRotationTimeRange)
{
	UAbilityTask_AbilityActionTimeline* Task = NewAbilityTask<UAbilityTask_AbilityActionTimeline>(OwningAbility);
	Task->bTickingTask = true;
	Task->Actions = Actions;
	Task->ActionContext = ActionContext;
	Task->LatencyCompensation = FMath::Max(LatencyCompensation, 0.0f);
	Task->BlockMovementTimeRange = BlockMovementTimeRange;
	Task->BlockRotationTimeRange = BlockRotationTimeRange;
	return Task;
}

void UAbilityTask_AbilityActionTimeline::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	if (bIsFinished)
	{
		return;
	}
	
	const double CurrentSeconds = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	
	UpdateActions(CurrentSeconds);
	UpdateBlock(CurrentSeconds);
}

void UAbilityTask_AbilityActionTimeline::OnDestroy(bool bInOwnerFinished)
{
	bIsFinished = true;
	
	UnblockMovement();
	UnblockRotation();
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_AbilityActionTimeline::Activate()
{
	Super::Activate();
	
	const int ActionCount = Actions.Num();
	if (ActionCount < 1)
	{
		FinishTask(true);
		return;
	}
	
	const double CurrentSeconds = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	ActivationWorldSeconds = CurrentSeconds;
	ActivationWorldSeconds -= LatencyCompensation; // 보정 적용
	
	ActionStates.Reserve(ActionCount);
	// AsyncTaskMap은 전부 사용하진 않지만 최대치는 정해져있음
	AsyncTaskMap.Reserve(ActionCount);
	
	for (int i = 0; i < ActionCount; ++i)
	{
		const FURAbilityActionBase* AbilityAction = Actions[i].GetPtr<FURAbilityActionBase>();
		
		if (!AbilityAction || !AbilityAction->HasValidData())
		{
			// 데이터 오류 실패 정책 상관없이 즉시 종료
			FinishTask(false);
			return;
		}
		
		ActionStates.Add(EURAbilityActionState::Pending);
	}
	
	// 시작 시점에 실행 시간이 된 액션들 즉시 시작할 수 있도록 Update
	UpdateActions(CurrentSeconds);
	UpdateBlock(CurrentSeconds);
}

UURGameplayAbility_ActiveAbility* UAbilityTask_AbilityActionTimeline::GetURAbility() const
{
	return Cast<UURGameplayAbility_ActiveAbility>(Ability);
}

void UAbilityTask_AbilityActionTimeline::UpdateActions(const double CurrentSeconds)
{
	if (bIsFinished)
	{
		return;
	}
	
	const int ActionCount = Actions.Num();
	const double TimelineElapsed = CurrentSeconds - ActivationWorldSeconds;
	bool bIsDirty = false;
	for (int i = 0; i < ActionCount; i++)
	{
		if (bIsFinished)
		{
			// 진행 중 처리되어 Finished되었을 경우 이후 처리 하지 않음 
			return;
		}
		
		if (ActionStates[i] != EURAbilityActionState::Pending)
		{
			// 대기 중 아님
			continue;
		}
		
		const FURAbilityActionBase* AbilityAction = Actions[i].GetPtr<FURAbilityActionBase>();
		
		if (CurrentSeconds < ActivationWorldSeconds + AbilityAction->StartTime)
		{
			// 시작 시간 안 됨
			continue;
		}
		
		
		// 태스크 시작
		const FURAbilityActionStartResult StartResult = AbilityAction->TryStartAbilityAction(GetURAbility(), ActionContext, TimelineElapsed);
		
		if (StartResult.State == EURAbilityActionStartState::Completed)
		{
			// 즉시 완료됨
			ActionStates[i] = EURAbilityActionState::Completed;
			bIsDirty = true;
			continue;
		}
		
		if (StartResult.State == EURAbilityActionStartState::Failed)
		{
			// 실패
			if (AbilityAction->FailedPolicy == EAbilityActionFailedPolicy::AbortTimeline)
			{
				// 실패 정책이 타임라인 종료일 경우 종료
				FinishTask(false);
				return;
			}
			
			ActionStates[i] = EURAbilityActionState::Continued;
			bIsDirty = true;
			continue;
		}
		
		if (StartResult.State == EURAbilityActionStartState::Running)
		{
			// 비동기 액션이 실행됨
			if (!StartResult.Task)
			{
				// Running이지만 Task가 유효하지 않음
				if (AbilityAction->FailedPolicy == EAbilityActionFailedPolicy::Type::AbortTimeline)
				{
					// 실패 정책이 타임라인 종료일 경우 종료
					FinishTask(false);
					return;
				}
				
				ActionStates[i] = EURAbilityActionState::Continued;
				bIsDirty = true;
				continue;
			}
			
			// 비동기 Task 시작됨
			ActionStates[i] = EURAbilityActionState::Running;
			AsyncTaskMap.Add(StartResult.Task, i);
			StartResult.Task->OnCompleted.AddDynamic(this, &ThisClass::HandleTaskCompleted);
			StartResult.Task->OnFailed.AddDynamic(this, &ThisClass::HandleTaskFailed);
			StartResult.Task->ReadyForActivation();
		}
	}
	
	if (bIsDirty && !bIsFinished)
	{
		// 변경 있으면 종료 검사
		CheckFinish();
	}
}

void UAbilityTask_AbilityActionTimeline::UpdateBlock(const double CurrentSeconds)
{
	if (bIsFinished)
	{
		return;
	}
	
	const float Elapsed = CurrentSeconds - ActivationWorldSeconds;
	if (MovementBlockState < EURBlockState::Done)
	{
		const bool bIsEnded =
				BlockMovementTimeRange.HasUpperBound() &&
				(BlockMovementTimeRange.GetUpperBound().IsExclusive()
					? Elapsed >= BlockMovementTimeRange.GetUpperBoundValue()
					: Elapsed > BlockMovementTimeRange.GetUpperBoundValue());
		
		if (BlockMovementTimeRange.IsEmpty() ||
			!BlockMovementTimeRange.HasLowerBound() ||
			bIsEnded)
		{
			UnblockMovement();
		}
		else if (MovementBlockState == EURBlockState::NotSet && BlockMovementTimeRange.Contains(Elapsed))
		{
			// 이동 제한
			BlockMovement();
		}
	}
	
	if (RotationBlockState < EURBlockState::Done)
	{
		const bool bIsEnded =
				BlockRotationTimeRange.HasUpperBound() &&
				(BlockRotationTimeRange.GetUpperBound().IsExclusive()
					? Elapsed >= BlockRotationTimeRange.GetUpperBoundValue()
					: Elapsed > BlockRotationTimeRange.GetUpperBoundValue());
		
		if (BlockRotationTimeRange.IsEmpty() ||
			!BlockRotationTimeRange.HasLowerBound() ||
			bIsEnded)
		{
			UnblockRotation();
		}
		else if (RotationBlockState == EURBlockState::NotSet && BlockRotationTimeRange.Contains(Elapsed))
		{
			// 이동 제한
			BlockRotation();
		}
	}
}

void UAbilityTask_AbilityActionTimeline::BlockMovement()
{
	MovementBlockState = EURBlockState::Set;
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(URGameplayTags::Player_State_InputBlocked_Movement);
	}
}

void UAbilityTask_AbilityActionTimeline::UnblockMovement()
{
	if (MovementBlockState == EURBlockState::Set)
	{
		// 추가했을 경우에만 제거
		if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
		{
			ASC->RemoveLooseGameplayTag(URGameplayTags::Player_State_InputBlocked_Movement);
		}
	}
	
	MovementBlockState = EURBlockState::Done;
	CheckFinish();
}

void UAbilityTask_AbilityActionTimeline::BlockRotation()
{
	RotationBlockState = EURBlockState::Set;
	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(URGameplayTags::Character_State_Blocked_PhysicsRotation);
	}
}

void UAbilityTask_AbilityActionTimeline::UnblockRotation()
{
	if (RotationBlockState == EURBlockState::Set)
	{
		// 추가했을 경우에만 제거
		if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
		{
			ASC->RemoveLooseGameplayTag(URGameplayTags::Character_State_Blocked_PhysicsRotation);
		}
	}
	
	RotationBlockState = EURBlockState::Done;
	CheckFinish();
}

void UAbilityTask_AbilityActionTimeline::HandleTaskCompleted(UAbilityActionTask* Task)
{
	if (bIsFinished)
	{
		return;
	}
	
	const int32* Index = AsyncTaskMap.Find(Task);
	if (!Index)
	{
		// 이미 종료되었거나, 등록되지 않은 Task
		return;
	}
	
	ActionStates[*Index] = EURAbilityActionState::Completed;
	AsyncTaskMap.Remove(Task);
	
	CheckFinish();
	
}

void UAbilityTask_AbilityActionTimeline::HandleTaskFailed(UAbilityActionTask* Task)
{
	if (bIsFinished)
	{
		return;
	}
	
	const int32* Index = AsyncTaskMap.Find(Task);
	if (!Index)
	{
		// 이미 종료되었거나, 등록되지 않은 Task
		return;
	}

	ActionStates[*Index] = EURAbilityActionState::Continued;
	const FURAbilityActionBase* AbilityAction = Actions[*Index].GetPtr<FURAbilityActionBase>();
	AsyncTaskMap.Remove(Task);
	if (AbilityAction->FailedPolicy == EAbilityActionFailedPolicy::Type::AbortTimeline)
	{
		// 실패 정책이 타임라인 종료일 경우 종료
		FinishTask(false);
		return;
	}
	
	CheckFinish();
}

void UAbilityTask_AbilityActionTimeline::CheckFinish()
{
	if (bIsFinished)
	{
		return;
	}
	
	// bool bIsSuccess = true;
	for (const EURAbilityActionState& State : ActionStates)
	{
		// Note: 실패 정책이 Abort일 경우 CheckFinish 거치지 않고 즉시 종료되고, Continue일 경우엔 성공으로 유지함
		// if (State == EURAbilityActionState::Continued)// || State == EURAbilityActionState::None)
		// {
		// 	// Failed나 None인 액션 있음
		// 	bIsSuccess = false;
		// 	continue;
		// }
		
		if (State == EURAbilityActionState::Completed || State == EURAbilityActionState::Continued)
		{
			continue;
		}
		
		// Pending이나 Running인 액션 있으면 종료하지 않음
		return;
	}

	if (MovementBlockState < EURBlockState::Done && BlockMovementTimeRange.GetUpperBound().IsClosed())
	{
		return;
	}
	if (RotationBlockState < EURBlockState::Done && BlockRotationTimeRange.GetUpperBound().IsClosed())
	{
		return;
	}
	
	FinishTask(true);
}

void UAbilityTask_AbilityActionTimeline::FinishTask(bool bIsSuccess)
{
	if (bIsFinished)
	{
		return;
	}
	
	bIsFinished = true;
	bTickingTask = false;
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bIsSuccess)
		{
			OnCompleted.Broadcast();
		}
		else
		{
			OnFailed.Broadcast();
		}
	}
	
	EndTask();
}
