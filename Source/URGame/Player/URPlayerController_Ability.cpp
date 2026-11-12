// Written by EXPx2

#include "Player/URGamePlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemInterface.h"
#include "URGame.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Character/URTargetingComponent.h"
#include "Components/URPathFollowingComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTags/URGameplayTags.h"
#include "Navigation/PathFollowingComponent.h"
#include "UI/URUISubsystem.h"

bool AURGamePlayerController::TryActivateAbility(FGameplayAbilitySpecHandle AbilityToActivate)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ensure(ASC))
	{
		return false;
	}
	
	const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(AbilityToActivate);
	UURGameplayAbility_ActiveAbility* ActiveAbility = Spec ? Cast<UURGameplayAbility_ActiveAbility>(Spec->Ability) : nullptr;
	if (!Spec || !ActiveAbility)
	{
		// Spec이 없을 경우 기본 구현의 실패 로그 남기기 위해 기본 구조 호출
		// URGameplayAbility_Ability(스킬) 아닐 경우 기본 구조대로 활성화
		return ASC->TryActivateAbility(AbilityToActivate);
	}
		
	// 사용 대기 중이던 어빌리티 취소
	ClearPendingAbility();
	
	FGameplayTagContainer FailureTags;
	if (!ActiveAbility->CanActivateAbility(
				AbilityToActivate,
				ASC->AbilityActorInfo.Get(),
				&ASC->GetOwnedGameplayTags(),
				nullptr,
				&FailureTags))
	{
		// 사용자 조건만으로 사용 불가
		ASC->NotifyAbilityFailed(AbilityToActivate, ActiveAbility, FailureTags);
		return false;
	}
	
	
	const EAbilityTargetingPolicy::Type TargetingPolicy = ActiveAbility->GetTargetingPolicy();
	
	// 타겟이 필요 없는 스킬도 대상 우선순위로 설정하기 위해 타겟 정보 보냄
	
	// 타겟 지정
	TArray<AActor*> TargetActors;
	if (IsValid(TargetingComponent))
	{
		if (AActor* MainTargetActor = TargetingComponent->GetMainTargetActor())
		{
			TargetActors.Add(MainTargetActor);
		}

		if (AActor* AimTargetActor = TargetingComponent->GetAimTargetActor())
		{
			TargetActors.Add(AimTargetActor);
		}
	}
	
	AActor* TargetActor = nullptr;
	
	for (AActor* Actor : TargetActors)
	{
		if (ActiveAbility->IsValidTarget(ASC->GetOwnerActor(), Actor))
		{
			// 사용 거리를 제외한 어빌리티 실행 가능 여부 검사하여 어빌리티 사용 등록
			if (ActiveAbility->CanActivateAbilityToTarget(
				ASC,
				AbilityToActivate,
				Actor))
			{
				// 사용 가능
				TargetActor = Actor;
				break;
			}
		}
		
		if (TargetingPolicy == EAbilityTargetingPolicy::Type::Targeting)
		{
			// 타겟팅 스킬은 첫번째 타겟이 유효하지 않으면 사용 불가능 처리
			ASC->NotifyAbilityFailed(AbilityToActivate, ActiveAbility, FGameplayTagContainer(URGameplayTags::TAG_Ability_Fail_InvalidTarget));
			return false;
		}
	}
	
	if (TargetActor)
	{
		// 타겟이 유효함
		if (TargetingPolicy == EAbilityTargetingPolicy::Type::Range ||
			TargetingPolicy == EAbilityTargetingPolicy::Type::NonTargeting ||
			IsTargetInRange(ActiveAbility, TargetActor))
		{
			// 타겟을 우선 순위 대상으로만 사용하는 타겟팅 정책이거나,
			// 타겟이 이미 범위 안에 있으면 즉시 Trigger
			return TriggerAbility(AbilityToActivate, TargetActor);
		}

		// 자동 이동 후 어빌리티 사용하도록 Pending에 등록
		return SetPendingAbilityAndMoveToTarget(AbilityToActivate, TargetActor);
	}
	
	// 타겟이 없음
	if (TargetingPolicy == EAbilityTargetingPolicy::Type::Targeting)
	{
		// 타겟팅 스킬은 첫번째 타겟이 유효하지 않으면 사용 불가능 처리
		ASC->NotifyAbilityFailed(AbilityToActivate, ActiveAbility, FGameplayTagContainer(URGameplayTags::TAG_Ability_Fail_NoTarget));
		return false;
	}
	
	return TriggerAbility(AbilityToActivate, nullptr);
}

bool AURGamePlayerController::IsTargetInRange(const UURGameplayAbility_ActiveAbility* ActiveAbility, const AActor* Actor) const
{
	/*
	 * 타겟의 Radius를 거리에 포함시켜 범위안에 걸쳐도 InRange로 판단함
	 * HasReached 구현을 보면 INavAgentInterface일 경우에만 대상의 Radius를 포함 시켜 다른 액터에도 대응되도록 함.
	 */
	const float TargetRadius = Actor->GetSimpleCollisionRadius();
	
	return PathFollowingComponent->HasReached(*Actor, EPathFollowingReachMode::ExactLocation, ActiveAbility->GetUseRange(HasAuthority() && !IsLocalController()) + TargetRadius);
}

void AURGamePlayerController::ClientAbilityFailed_Implementation(
	const FGameplayAbilitySpecHandle SpecHandle,
	const int16 ActivationPredictionKey,
	const FGameplayTagContainer& FailureReason)
{
	LocalAbilityFailed(SpecHandle, ActivationPredictionKey, FailureReason);
}

void AURGamePlayerController::LocalAbilityFailed(
	const FGameplayAbilitySpecHandle SpecHandle,
	const int16 ActivationPredictionKey,
	const FGameplayTagContainer& FailureReason)
{
	/*
	 * Note:
	 * 클라이언트와 서버에서 모두 실패하면 같은 내용의 토스트 메세지가 중복으로 표시되는 등의 문제가 생긴다.
	 * 동일 어빌리티의 실행 실패는 중복으로 처리하지 않는다.
	 */
	
	
	if (!IsLocalController())
	{
		return;
	}
	
	// 중복 검사하여 이미 처리한 어빌리티 실행일 경우 중복 호출 방지 
	const double CurrentTime = GetWorld()->GetTimeSeconds();
		
	// 오래된 기록 삭제
	FailedAbilityRecords.RemoveAllSwap([CurrentTime](const FURAbilityFailedRecord& Record)
	{
		return CurrentTime > Record.Time + 1.0;
	});
		
	for (FURAbilityFailedRecord& Record : FailedAbilityRecords)
	{
		if (Record.Handle == SpecHandle && Record.PredictionKey == ActivationPredictionKey)
		{
			// 중복 (이미 클라이언트에서 처리됨)
			return;
		}
	}
	
	FailedAbilityRecords.Add(FURAbilityFailedRecord{SpecHandle, ActivationPredictionKey, CurrentTime});
	
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UURUISubsystem* UISystem = LP->GetSubsystem<UURUISubsystem>())
		{
			UISystem->AbilityFailedToastMessage(FailureReason);
		}
	}
}

UAbilitySystemComponent* AURGamePlayerController::GetAbilitySystemComponent() const
{
	if (APlayerState* PS = GetPlayerState<APlayerState>())
	{
		if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(PS))
		{
			return ASI->GetAbilitySystemComponent();
		}
	}
	
	if (APawn* Pwn = GetPawn())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pwn))
		{
			return ASC;
		}
	}
	
	return nullptr;
}

bool AURGamePlayerController::TriggerAbility(
	FGameplayAbilitySpecHandle AbilityToActivate, 
	AActor* TargetActor) const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}
	
	return UURGameBlueprintLibrary::TriggerAbilityWithTarget(ASC, AbilityToActivate, TargetActor);
	
	// UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	//
	// FGameplayEventData EventData;
	// EventData.Instigator = ASC->GetOwnerActor();
	// EventData.InstigatorTags = ASC->GetOwnedGameplayTags();
	// EventData.Target = TargetActor;
	// EventData.TargetTags = TargetASC ? TargetASC->GetOwnedGameplayTags() : FGameplayTagContainer();
	//
	// return ASC->TriggerAbilityFromGameplayEvent(
	// 	AbilityToActivate,
	// 	ASC->AbilityActorInfo.Get(),
	// 	FGameplayTag(),
	// 	&EventData,
	// 	*ASC);
}

bool AURGamePlayerController::SetPendingAbilityAndMoveToTarget(
	FGameplayAbilitySpecHandle AbilityToActivate, 
	AActor* TargetActor)
{
	TargetActor = GetValid(TargetActor);
	
	if (!ensure(TargetActor))
	{
		UE_LOG(LogUR, Error, TEXT("%hs Failed to set PendingAbility cause target actor is null"), __func__)
		return false;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	check(ASC);
	
	// 자동 이동 시작
	FAIRequestID RequestID;
	PathFollowingComponent->MoveToActor(this, TargetActor, RequestID);
	
	if (RequestID.IsValid())
	{
		PendingAbility.MoveRequestID = RequestID;
		PendingAbility.AbilitySystemComponent = ASC;
		PendingAbility.Pawn = GetPawn();
		PendingAbility.AbilityToActivate = AbilityToActivate;
		PendingAbility.TargetActor = TargetActor;
		
		/*
		 * Note: 서버에서의 유효성 검사
		 * PendingAbility를 설정하고 이동 시작 시에 1회 검사한다.
		 * 이동 중 검사는 클라이언트에서 복제된 값을 통해 클라이언트에서만 검사한다.
		 * => 유효하지 않은 대상으로 접근을 할 수는 있지만 도착 후 어빌리티 사용은 어빌리티에서 활성화 검사를 하기 때문에 비정상 어빌리티 활성화는 되지 않음.
		 */
		if (!HasAuthority() && IsLocalController())
		{
			ServerSetPendingAbility(RequestID, AbilityToActivate, TargetActor);
		}
		
		/*
		 * Note: 전투 대상 바라보는 중에 다른 대상에게 스킬 사용하러 이동할 때 기존 전투 대상 바라봄 (아이온2)
		 * 회전 동기화를 위해 서버에서도 PendingAbility 설정했지만, 필요 없어져 설정하지 않음
		 */
		// if (UURCharacterMovementComponent* MovementComponent = Cast<UURCharacterMovementComponent>(GetPawn()->GetMovementComponent()))
		// {
		// 	const FURRotationContext Context(TargetActor);
		// 	MovementComponent->RotationResolver.Set(EURRotationSource::MoveTo, Context);
		// }
		
		return true;
	}
	else
	{
		// 자동 이동 실패
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(AbilityToActivate);
		ASC->NotifyAbilityFailed(AbilityToActivate, Spec ? Spec->Ability : nullptr, FGameplayTagContainer(URGameplayTags::TAG_Ability_Fail_MoveFailed));
		return false;
	}
}

void AURGamePlayerController::ServerSetPendingAbility_Implementation(
	int32 RequestID,
	const FGameplayAbilitySpecHandle AbilityToActivate, 
	AActor* TargetActor)
{
	FGameplayTagContainer FailureTags;
	if (AbilityToActivate.IsValid() && IsValid(TargetActor))
	{
		if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(AbilityToActivate))
			{
				if (const UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Spec->Ability))
				{
					if (ActiveAbility->IsValidTarget(ASC->GetOwnerActor(), TargetActor))
					{
						// 사용 거리를 제외한 어빌리티 실행 가능 여부 검사하여 어빌리티 사용 등록
						if (ActiveAbility->CanActivateAbilityToTarget(
							ASC,
							AbilityToActivate,
							TargetActor,
							&FailureTags))
						{
							// 사용 가능
							/*
							 * Note: 전투 대상 바라보는 중에 다른 대상에게 스킬 사용하러 이동할 때 기존 전투 대상 바라봄 (아이온2)
							 * 회전 동기화를 위해 서버에서도 PendingAbility 설정했지만, 필요 없어져 설정하지 않음
							 */
							// PendingAbility.MoveRequestID = RequestID;
							// PendingAbility.AbilityToActivate = AbilityToActivate;
							// PendingAbility.TargetActor = TargetActor;
							
							// if (UURCharacterMovementComponent* MovementComponent = Cast<UURCharacterMovementComponent>(GetPawn()->GetMovementComponent()))
							// {
							// 	const FURRotationContext Context(TargetActor);
							// 	MovementComponent->RotationResolver.Set(EURRotationSource::MoveTo, Context);
							// }
							return;
						}
					}
				}
			}
		}
	}
	
	// 서버에서의 검증 실패로 중지하도록 RPC 전송
	ClientKillPendingAbility(AbilityToActivate, FailureTags);
}

void AURGamePlayerController::ClientKillPendingAbility_Implementation(
	const FGameplayAbilitySpecHandle AbilityToKill,
	const FGameplayTagContainer Reason)
{
	if (PendingAbility.AbilityToActivate != AbilityToKill)
	{
		// 서버에서 검증 후 Kill 전송 전에 PendingAbility 변경
		return;
	}
	
	/*
	 * Note: Handle만을 기준으로 검사하기 때문에, 같은 어빌리티일 경우 지연된 실패 RPC가 새 요청을 취소할 수 있음.
	 * RequestID를 추가해 개선 가능하지만, 지연 시간 동안 조건이 변경될 가능성이 크지 않고, 개선 시 지연 시간 동안 계속 새 요청을 하면 실패가 안 될 가능성 있음
	 * => 이전 요청 실패 시 현재 요청도 실패하도록 유지함.
	 */
	if (UAbilitySystemComponent* ASC = PendingAbility.AbilitySystemComponent.Get())
	{
		if (const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(AbilityToKill))
		{
			ASC->NotifyAbilityFailed(AbilityToKill, Spec->Ability, Reason);
		}
	}
	
	ClearPendingAbility();
}

void AURGamePlayerController::CheckPendingAbilityToActivate()
{
	if (!PendingAbility.AbilityToActivate.IsValid())
	{
		// PendingAbility 없음
		return;
	}
	
	if (IsLocalController())
	{
		const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		AActor* TargetActor = PendingAbility.TargetActor.Get();
		const FGameplayAbilitySpec* Spec =  ASC ? ASC->FindAbilitySpecFromHandle(PendingAbility.AbilityToActivate) : nullptr;
		const UURGameplayAbility_ActiveAbility* ActiveAbility = Spec ? Cast<UURGameplayAbility_ActiveAbility>(Spec->Ability) : nullptr;
		if (!ASC ||
			!TargetActor ||
			!Spec ||
			!ActiveAbility ||
			// Pending 설정 할 때의 ASC와 폰이 바뀌었을 경우
			ASC != PendingAbility.AbilitySystemComponent.Get() ||
			!GetPawn() || GetPawn() != PendingAbility.Pawn.Get())
		{
			// ASC나 폰이 바뀌었거나, 어빌리티가 유효하지 않음
			ClearPendingAbility();
			return;
		}
	
	
		if (!ActiveAbility->IsValidTarget(ASC->GetOwnerActor(), TargetActor))
		{
			ClearPendingAbility();
			// 이동 중 상태 변경되어 취소될 때는 Notify하지 않음
			// ASC->NotifyAbilityFailed(PendingAbility.AbilityToActivate, Ability, FGameplayTagContainer(URGameplayTags::TAG_Ability_Fail_InvalidTarget));
			return;
		}
		
		if (!ActiveAbility->CanActivateAbilityToTarget(
			ASC,
			PendingAbility.AbilityToActivate,
			TargetActor))
		{
			// 이동 중 상태 변경되어 취소될 때는 Notify하지 않음
			ClearPendingAbility();
			return;
		}
	
		if (IsTargetInRange(ActiveAbility, TargetActor))
		{
			// 범위 안에 있으면 Trigger
			TriggerAbility(PendingAbility.AbilityToActivate, TargetActor);
		
			ClearPendingAbility();
		}
	}
	else if (!HasAuthority())
	{
		// 로컬도 아니고 서버도 아닐 경우 처리하지 않는다.
		return;
	}
}

void AURGamePlayerController::ClearPendingAbility()
{
	if (PendingAbility.IsValid())
	{
		PendingAbility.AbilitySystemComponent.Reset();
		PendingAbility.AbilityToActivate = FGameplayAbilitySpecHandle();
		PendingAbility.TargetActor.Reset();

		if (IsLocalController())
		{
			// RequestFinished에서 PendingAbility를 이용해 호출하지 않도록 비운 뒤에 호출
			if (PendingAbility.Pawn.Get() == GetPawn())
			{
				PathFollowingComponent->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, PendingAbility.MoveRequestID);
			}
			// 서버에서 PendingAbility 사용하지 않도록 수정함
			// if (!HasAuthority())
			// {
			// 	ServerClearPendingAbility(PendingAbility.MoveRequestID);
			// }
			PendingAbility.MoveRequestID = 0;
		}
		
		// if (UURCharacterMovementComponent* MovementComponent = Cast<UURCharacterMovementComponent>(GetPawn()->GetMovementComponent()))
		// {
		// 	MovementComponent->RotationResolver.CancelSource(EURRotationSource::MoveTo);
		// }
	}
}

void AURGamePlayerController::ServerClearPendingAbility_Implementation(const int32 RequestID)
{
	if (PendingAbility.MoveRequestID == RequestID)
	{
		ClearPendingAbility();
	}
}

void AURGamePlayerController::HandleMoveRequestFinished(
	FAIRequestID RequestID,
	const FPathFollowingResult& PathFollowingResult)
{
	// 이동 완료 전에 거리 검사로 어빌리티를 트리거하고, 이동을 Abort 시키므로 실행 가능성 적음
	if (PendingAbility.MoveRequestID == RequestID)
	{
		if (PathFollowingResult.IsSuccess())
		{
			// Pending 중인 어빌리티 있을 경우 사용 시도
			if (PendingAbility.AbilityToActivate.IsValid())
			{
				TriggerAbility(PendingAbility.AbilityToActivate, PendingAbility.TargetActor.Get());
			}
		}
		ClearPendingAbility();
	}
}
