// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityTask_LookTarget.h"

#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Character/URCharacterMovementComponent.h"

UAbilityTask_LookTarget* UAbilityTask_LookTarget::LookTarget(
	UGameplayAbility* OwningAbility, 
	AActor* TargetActor)
{
	UAbilityTask_LookTarget* Task = NewAbilityTask<UAbilityTask_LookTarget>(OwningAbility);
	Task->bTickingTask = true;
	Task->TargetActorPtr = TargetActor;
	return Task;
}

UAbilityTask_LookTarget* UAbilityTask_LookTarget::LookTarget(
	UGameplayAbility* OwningAbility,
	const FVector& InTargetLocation)
{
	UAbilityTask_LookTarget* Task = NewAbilityTask<UAbilityTask_LookTarget>(OwningAbility);
	Task->bTickingTask = true;
	Task->TargetActorPtr = nullptr;
	Task->TargetLocation = InTargetLocation;
	return Task;
}

void UAbilityTask_LookTarget::OnDestroy(bool bInOwnerFinished)
{
	TargetActorPtr.Reset();
	
	if (MovementComponent.IsValid())
	{
		MovementComponent->RotationResolver.CancelSource(EURRotationSource::Ability);
	}
	MovementComponent.Reset();
	
	Super::OnDestroy(bInOwnerFinished);
}


void UAbilityTask_LookTarget::Activate()
{
	Super::Activate();
	
	MovementComponent = GetAvatarActor()->FindComponentByClass<UURCharacterMovementComponent>();
	
	if (!MovementComponent.IsValid())
	{
		EndTask();
		return;
	}
	
	FVector LookAtLocation = TargetLocation;
	if (TargetActorPtr.IsValid())
	{
		FURRotationContext Context(TargetActorPtr.Get());
		Context.bUsingRotationSpeed = false;
		MovementComponent->RotationResolver.Set(EURRotationSource::Ability, Context);
		
		LookAtLocation = UURGameBlueprintLibrary::GetFeetLocation(TargetActorPtr.Get());
	}
	else
	{
		FURRotationContext Context(TargetLocation);
		Context.bUsingRotationSpeed = false;
		MovementComponent->RotationResolver.Set(EURRotationSource::Ability, Context);
	}

	MovementComponent->LookAt(LookAtLocation, false);
}
