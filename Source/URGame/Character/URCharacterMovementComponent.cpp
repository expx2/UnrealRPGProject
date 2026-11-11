// Written by EXPx2


#include "Character/URCharacterMovementComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameplayTags/URGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"

UURCharacterMovementComponent::UURCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NavMovementProperties.bUseAccelerationForPaths = true;
}

void UURCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		// 회전 제한
		if (ASC->HasMatchingGameplayTag(URGameplayTags::Character_State_Blocked_PhysicsRotation))
		{
			return;
		}
	}
	
	const FURRotationContext* CurrentContext = RotationResolver.GetCurrentValue();
	if (!CurrentContext)
	{
		bOrientRotationToMovement = true;
		Super::PhysicsRotation(DeltaTime);
		return;
	}
	
	if (!HasValidData() || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController))
	{
		return;
	}

	// Super::PhysicsRotation 호출을 안 하기 때문에 bOrientRotationToMovement 설정하지 않아도 괜찮지만, ABP에서 사용하기 때문에 설정함
	bOrientRotationToMovement = false;
	// Set the new rotation.
	if (CurrentContext->bHasTargetActor && !CurrentContext->TargetActor.IsValid())
	{
		// 타겟을 바라보는 값인데, 타겟이 유효하지 않을 경우 현재 소스 캔슬
		RotationResolver.CancelSource(RotationResolver.GetCurrentSource());
		return;
	}
	
	LookAt(CurrentContext->GetLookAtLocation(), CurrentContext->bUsingRotationSpeed, DeltaTime);
}

void UURCharacterMovementComponent::LookAt(
	const FVector& Location,
	const bool bUsingRotationSpeed,
	float DeltaSeconds)
{
	if (DeltaSeconds < 0.f)
	{
		DeltaSeconds = GetWorld()->GetDeltaSeconds();
	}
	
	FRotator DesiredRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), Location);
	DesiredRotation.Pitch = 0.f;
	DesiredRotation.Roll = 0.f;
	
	if (bUsingRotationSpeed)
	{
		// UCharacterMovementComponent::PhysicsRotation 참고 속도에 따라 회전
		const FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		const FRotator DeltaRot = GetDeltaRotation(DeltaSeconds);
		const float AngleTolerance = 1e-3f;
		if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
		{
			DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
		}
	}
	MoveUpdatedComponent( FVector::ZeroVector, DesiredRotation, /*bSweep*/ false );
}