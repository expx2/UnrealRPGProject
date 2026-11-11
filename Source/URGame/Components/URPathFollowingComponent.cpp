// Written by EXPx2


#include "URPathFollowingComponent.h"

#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

#define LOCTEXT_NAMESPACE "URPathFollowingComponent"

void UURPathFollowingComponent::MoveToActor(AController* Controller, const AActor* Goal, FAIRequestID& OutRequestID)
{
	// UAIBlueprintHelperLibrary::SimpleMoveToActor 복사해서 사용함 
	// RequestID를 반환하여 해당 요청에 대한 RequestID를 보장함
	OutRequestID = FAIRequestID::InvalidRequest;
	
	UNavigationSystemV1* NavSys = Controller ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(Controller->GetWorld()) : nullptr;
	if (NavSys == nullptr || Goal == nullptr || Controller == nullptr || Controller->GetPawn() == nullptr)
	{
		UE_LOG(LogNavigation, Warning, TEXT("UNavigationSystemV1::SimpleMoveToActor called for NavSys:%s Controller:%s controlling Pawn:%s with goal actor %s (if any of these is None then there's your problem"),
			*GetNameSafe(NavSys), *GetNameSafe(Controller), Controller ? *GetNameSafe(Controller->GetPawn()) : TEXT("NULL"), *GetNameSafe(Goal));
		return;
	}

	// UPathFollowingComponent* PFollowComp = InitNavigationControl(*Controller);
	UPathFollowingComponent* PFollowComp = this;

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

	const bool bAlreadyAtGoal = PFollowComp->HasReached(*Goal, EPathFollowingReachMode::OverlapAgentAndGoal);

	// script source, keep only one move request at time
	if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
	{
		PFollowComp->AbortMove(*NavSys, FPathFollowingResultFlags::ForcedScript | FPathFollowingResultFlags::NewRequest
			, FAIRequestID::AnyRequest, bAlreadyAtGoal ? EPathFollowingVelocityMode::Reset : EPathFollowingVelocityMode::Keep);
	}

	if (bAlreadyAtGoal)
	{
		OutRequestID = PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
	}
	else
	{
		const FVector AgentNavLocation = Controller->GetNavAgentLocation();
		const ANavigationData* NavData = NavSys->GetNavDataForProps(Controller->GetNavAgentPropertiesRef(), AgentNavLocation);
		if (NavData)
		{
			FPathFindingQuery Query(Controller, *NavData, AgentNavLocation, Goal->GetActorLocation());
			FPathFindingResult Result = NavSys->FindPathSync(Query);
			if (Result.IsSuccessful())
			{
				Result.Path->SetGoalActorObservation(*Goal, 100.0f);
				OutRequestID = PFollowComp->RequestMove(FAIMoveRequest(Goal), Result.Path);
			}
			else if (PFollowComp->GetStatus() != EPathFollowingStatus::Idle)
			{
				OutRequestID = PFollowComp->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE