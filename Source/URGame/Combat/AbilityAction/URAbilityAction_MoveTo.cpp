// Written by EXPx2


#include "URAbilityAction_MoveTo.h"

#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_MoveTo.h"
#include "Kismet/KismetMathLibrary.h"
#include "URGame/AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"

FURAbilityActionStartResult FURAbilityAction_MoveTo::TryStartAbilityAction(
	UURGameplayAbility_ActiveAbility* Ability, 
	FURAbilityActionContext& InContext, 
	const float TimelineElapsed) const
{
	if (!HasValidData())
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	AActor* AvatarActor = Ability->GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return FURAbilityActionStartResult::Failed();
	}
	
	FVector TargetLocation;
	if (MoveToType == EMoveToType::Offset)
	{
		const FRotator AvatarYawRot(0.f, AvatarActor->GetActorRotation().Yaw, 0.f);
		TargetLocation = AvatarActor->GetActorLocation() + (AvatarYawRot.RotateVector(Offset));
	}
	else if (MoveToType == EMoveToType::ToTargetActor)
	{
		const AActor* TargetActor = InContext.TargetActorPtr.Get();
		if (!TargetActor)
		{
			return FURAbilityActionStartResult::Failed();
		}
		float AvatarRadius = 0.f;
		float AvatarHalfHeight = 0.f;
		AvatarActor->GetSimpleCollisionCylinder(AvatarRadius, AvatarHalfHeight);
	
		float TargetRadius = 0.f;
		float TargetHalfHeight = 0.f;
		TargetActor->GetSimpleCollisionCylinder(TargetRadius, TargetHalfHeight);
	
		TargetLocation = TargetActor->GetActorLocation();
		const FVector Delta = TargetLocation - AvatarActor->GetActorLocation();
		// 각자의 Radius만큼 거리에 추가해 타겟의 위치가 아니라 타겟 앞으로 이동
		TargetLocation -= Delta.GetSafeNormal2D() * (Distance + AvatarRadius + TargetRadius);
	
		// 둘의 높이가 다를 경우 같은 높이의 땅에 있어도 액터의 Z값이 다르기 때문에 타겟의 HalfHeight 뺀 다음 사용자의 HalfHeight를 더한다.
		TargetLocation.Z -= TargetHalfHeight;
		TargetLocation.Z += AvatarHalfHeight;
	}
	else if (MoveToType == EMoveToType::ToLocation)
	{
		const FVector Delta = InContext.Location - AvatarActor->GetActorLocation();
		TargetLocation = InContext.Location - Delta.GetSafeNormal2D() * Distance;
		
		// InContext.Location은 FeetLocation인데, MoveTo에서 사용하는 값은 ActorLocation이라 HalfHeight만큼 더해 줌
		const float AvatarHalfHeight = AvatarActor->GetSimpleCollisionHalfHeight();
		TargetLocation.Z += AvatarHalfHeight;
	}
	
	if (bLookGoal)
	{
		AvatarActor->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(AvatarActor->GetActorLocation(), TargetLocation));
	}
	
	UAbilityActionTask_MoveTo* MoveToActor =
		UAbilityActionTask_MoveTo::MoveToActor(
			Ability,
			TargetLocation,
			Duration,
			VectorCurve,
			GetActionElapsedTime(TimelineElapsed));
	
	return FURAbilityActionStartResult::Running(MoveToActor);
}
