// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_MoveTo.h"

#include "URGame.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Curves/CurveVector.h"
#include "GameFramework/GameStateBase.h"

UAbilityActionTask_MoveTo* UAbilityActionTask_MoveTo::MoveToActor(
	UURGameplayAbility_ActiveAbility* OwningAbility, 
	const FVector& TargetLocation,
	const float Duration,
	UCurveVector* VectorCurve,
	const float ElapsedTime)
{
	UAbilityActionTask_MoveTo* Task = NewAbilityTask<UAbilityActionTask_MoveTo>(OwningAbility);
	Task->bTickingTask = true;
	Task->TargetLocation = TargetLocation;
	Task->Duration = FMath::Max(Duration, 0.0f);
	Task->VectorCurve = VectorCurve;
	Task->ElapsedTime = FMath::Max(ElapsedTime, 0.0f);
	return Task;
}

void UAbilityActionTask_MoveTo::TickTask(float DeltaTime)
{
	if (bIsFinished)
	{
		return;
	}
	
	if (GetWorld()->GetGameState()->GetServerWorldTimeSeconds() >= EndTime)
	{
		// 종료 시간이 지났을 때 도착 여부 확인
		const bool bReachedDestination =
			FVector::DistSquared(GetAvatarActor()->GetActorLocation(), TargetLocation)
			<= FMath::Square(50.0f); // Tolerance

		FinishTask(bReachedDestination);
	}
}

void UAbilityActionTask_MoveTo::OnDestroy(bool bInOwnerFinished)
{
	if (UCharacterMovementComponent* CharacterMovementComponent = MovementComponent.Get())
	{
		CharacterMovementComponent->RemoveRootMotionSourceByID(RootMotionSourceID);
	}
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityActionTask_MoveTo::Activate()
{
	Super::Activate();
	
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	MovementComponent = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Character || !MovementComponent.IsValid())
	{
		FinishTask(false);
		return;
	}
	
	const float RemainDuration = Duration - ElapsedTime;
	
	if (RemainDuration <= SMALL_NUMBER)
	{
		// 이미 완료됨
		FHitResult HitResult;
		Character->SetActorLocation(
			TargetLocation,
			true,
			&HitResult,
			ETeleportType::None);
		
		const bool bReachedDestination =
			FVector::DistSquared(Character->GetActorLocation(), TargetLocation)
			<= FMath::Square(50.0f); // Tolerance

		FinishTask(bReachedDestination);
		return;
	}
	
	EndTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() + RemainDuration;
	
	TSharedPtr<FRootMotionSource_MoveToForce> MoveToSource = MakeShared<FRootMotionSource_MoveToForce>();
	MoveToSource->InstanceName = TEXT("AbilityActionTask_MoveToActor");
	MoveToSource->AccumulateMode = ERootMotionAccumulateMode::Override;
	// MoveToSource->Settings.SetFlag(ERootMotionSourceSettingsFlags::UseSensitiveLiftoffCheck);
	MoveToSource->Priority = 1000;
	MoveToSource->StartLocation = Character->GetActorLocation();
	MoveToSource->TargetLocation = TargetLocation;
	MoveToSource->Duration = Duration;
	MoveToSource->PathOffsetCurve = VectorCurve.Get();
	MoveToSource->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	MoveToSource->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	
	RootMotionSourceID = MovementComponent->ApplyRootMotionSource(MoveToSource);
	if (RootMotionSourceID == static_cast<uint16>(ERootMotionSourceID::Invalid))
	{
		// 이동 요청 후 ID가 Invalid
		FinishTask(false);
		return;
	}
	
	// 이동 진행률을 보정 시간만큼 앞으로 이동
	MoveToSource->SetTime(ElapsedTime);
	
	// if (UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo())
	// {
	// 	ASC->AddLooseGameplayTag(URGameplayTags::Character_State_AutoMove, 1);
	// 	bIsAddedAutoMoveTag = true;
	// }
	//
	// EndTime = UURUtilLibrary::GetServerWorldTimeSeconds(this) + FMath::Max(RemainDuration, 0.0f);
}

bool UAbilityActionTask_MoveTo::FinishTask(const bool bIsSuccess)
{
	if (!Super::FinishTask(bIsSuccess))
	{
		return false;
	}
	
	if (AActor* AvatarActor = GetAvatarActor())
	{
		AvatarActor->ForceNetUpdate();
	}

	return true;
}
