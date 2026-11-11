// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityTask_MoveToActor.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Navigation/PathFollowingComponent.h"

#define LOCTEXT_NAMESPACE "AbilityTask_MoveToActor"

UAbilityTask_MoveToActor* UAbilityTask_MoveToActor::MoveToActor(
	UGameplayAbility* OwningAbility,
	AActor* TargetActor)
{
	UAbilityTask_MoveToActor* Task = NewAbilityTask<UAbilityTask_MoveToActor>(OwningAbility);
	Task->bTickingTask = false;
	Task->TargetActorPtr = TargetActor;
	return Task;
}

void UAbilityTask_MoveToActor::OnDestroy(bool bInOwnerFinished)
{
	if (AActor* Actor = TargetActorPtr.Get())
	{
		Actor->OnEndPlay.RemoveDynamic(this, &ThisClass::HandleTargetEndPlay);
	}
	TargetActorPtr.Reset();
	
	if (UPathFollowingComponent* PathFollowingComponent = PathFollowingComponentPtr.Get())
	{
		if (MoveRequestFinishedDelegateHandle.IsValid())
		{
			PathFollowingComponent->OnRequestFinished.Remove(MoveRequestFinishedDelegateHandle);
		}

		if (MoveRequestId.IsValid())
		{
			// 이동이 현재 진행 중일 경우 이동 종료
			PathFollowingComponent->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, MoveRequestId);
		}
	}
	PathFollowingComponentPtr.Reset();
	MoveRequestFinishedDelegateHandle.Reset();
	MoveRequestId = FAIRequestID::InvalidRequest;

	// UURCharacterInputComponent::UpdateMovementInput 함수 내부 구현 참고
	// if (UURCharacterInputComponent* CharacterInputComponent = CharacterInputComponentPtr.Get())
	// {
	// 	CharacterInputComponent->OnMovementInputChanged.RemoveDynamic(this, &ThisClass::HandlePlayerMovementInputChanged);
	// }
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_MoveToActor::Activate()
{
	Super::Activate();
	
	AActor* Target = TargetActorPtr.Get();
	if (!Target)
	{
		FinishTask(false, false);
		return;
	}

	const APawn* AvatarPawn = Cast<APawn>(GetAvatarActor());
	if (!IsValid(AvatarPawn))
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
	
	SimpleMoveToActor(Controller, Target, MoveRequestId);
	
	if (!MoveRequestId.IsValid())
	{
		// 이동 요청 실패 혹은 이미 완료됨. IsFinished로 중복 종료 막음
		FinishTask(false, false);		
		return;
	}
	
	
	// 이동 종료
	if (PathFollowingComponentPtr.IsValid())
	{
		MoveRequestFinishedDelegateHandle = PathFollowingComponentPtr->OnRequestFinished.AddUObject(this, &ThisClass::HandleMoveRequestFinished);
	}
	
	// 타겟 액터 파괴되면 종료
	Target->OnEndPlay.AddDynamic(this, &ThisClass::HandleTargetEndPlay);
	
	// UURCharacterInputComponent::UpdateMovementInput 함수 내부 구현 참고
	// // 유저 이동 키 입력 시 종료
	// if (UURCharacterInputComponent* CharacterInputComponent = AvatarPawn->FindComponentByClass<UURCharacterInputComponent>())
	// {
	// 	CharacterInputComponentPtr = CharacterInputComponent;
	// 	CharacterInputComponent->OnMovementInputChanged.AddUniqueDynamic(this, &ThisClass::HandlePlayerMovementInputChanged);
	// }
	
	SetWaitingOnAvatar();
}

void UAbilityTask_MoveToActor::SimpleMoveToActor(
	AController* Controller, 
	const AActor* Goal,
	FAIRequestID& OutRequestID)
{
	// UAIBlueprintHelperLibrary::SimpleMoveToActor 복사해서 사용함 
	if (bIsFinished)
	{
		return;
	}
	
	UNavigationSystemV1* NavSys = Controller ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(Controller->GetWorld()) : nullptr;
	if (NavSys == nullptr || Goal == nullptr || Controller == nullptr || Controller->GetPawn() == nullptr)
	{
		UE_LOG(LogNavigation, Warning, TEXT("UNavigationSystemV1::SimpleMoveToActor called for NavSys:%s Controller:%s controlling Pawn:%s with goal actor %s (if any of these is None then there's your problem"),
			*GetNameSafe(NavSys), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"), *GetNameSafe(Goal));
		return;
	}

	UPathFollowingComponent* PFollowComp = UURGameBlueprintLibrary::GetPathFollowingComponent(Controller, true);

	if (PFollowComp == nullptr)
	{
		FMessageLog("PIE").Warning(FText::Format(
			LOCTEXT("SimpleMoveErrorNoComp", "SimpleMove failed for {0}: missing components"),
			FText::FromName(Controller->GetFName())
		));
		return;
	}

	if (!PFollowComp->IsPathFollowingAllowed())
	{
		FMessageLog("PIE").Warning(FText::Format(
			LOCTEXT("SimpleMoveErrorMovement", "SimpleMove failed for {0}: movement not allowed"),
			FText::FromName(Controller->GetFName())
		));
		return;
	}
	
	PathFollowingComponentPtr = PFollowComp;
	
	const bool bAlreadyAtGoal = PFollowComp->HasReached(*Goal, EPathFollowingReachMode::OverlapAgentAndGoal);

	// script source, keep only one move request at time
	if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		PFollowComp->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest
			, FAIRequestID::AnyRequest, bAlreadyAtGoal ? EPathFollowingVelocityMode::Reset : EPathFollowingVelocityMode::Keep);
	}

	if (bAlreadyAtGoal)
	{
		// RequestMoveWithImmediateFinish해도 OnRequestFinished가 호출되지만 RequestID가 달라서 FinishTask가 호출되지 않으므로 즉시 호출하고 종료함
		// PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		FinishTask(true, false);
		return;
	}
	else
	{
		const FVector AgentNavLocation = Controller->GetNavAgentLocation();
		const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef(), AgentNavLocation);
		if (NavData)
		{
			FPathFindingQuery Query(Controller, *NavData, AgentNavLocation, Goal->GetActorLocation());
			// 부분 경로 비허용
			Query.SetAllowPartialPaths(false);
			FPathFindingResult Result = NavSys->FindPathSync(Query);
			if (Result.IsSuccessful())
			{
				Result.Path->SetGoalActorObservation(*Goal, 100.0f);
				OutRequestID = PFollowComp->RequestMove(FAIMoveRequest(Goal), Result.Path);
			}
			else if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
			{
				PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
			}
		}
	}
}



void UAbilityTask_MoveToActor::HandleMoveRequestFinished(
	FAIRequestID RequestID,
	const FPathFollowingResult& Result)
{
	if (RequestID.IsEquivalent(MoveRequestId))
	{
		FinishTask(Result.IsSuccess(), Result.HasFlag(FPathFollowingResultFlags::UserAbort));
	}
	MoveRequestId = FAIRequestID::InvalidRequest;
}

void UAbilityTask_MoveToActor::HandleTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (Actor == TargetActorPtr.Get())
	{
		FinishTask(false, true);
	}
}

void UAbilityTask_MoveToActor::HandlePlayerMovementInputChanged(
	const FVector2D& OldInputValue,
	const FVector2D& NewInputValue)
{
	if (NewInputValue.IsNearlyZero())
	{
		// 이동 입력 취소는 무시함
		return;
	}

	FinishTask(false, true);
}

void UAbilityTask_MoveToActor::FinishTask(const bool bIsReached, const bool bIsCancelled)
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
			OnReached.Broadcast();
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

#undef LOCTEXT_NAMESPACE
