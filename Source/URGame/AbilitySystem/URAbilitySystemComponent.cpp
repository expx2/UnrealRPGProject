// Written by EXPx2


#include "AbilitySystem/URAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "AttributeSet/URHealthSet.h"
#include "GameplayAbility/URAbilityTagRelationshipMapping.h"
#include "GameplayAbility/URGameplayAbility.h"
#include "GameplayAbility/URGameplayAbilitySet.h"
#include "GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "GameplayEffect/URHitGameplayEffectContext.h"
#include "Player/URGamePlayerController.h"

namespace EAbilityInputBinds
{
	enum Type : int;
}

UURAbilitySystemComponent::UURAbilitySystemComponent()
{
	GenericConfirmInputID = EAbilityInputBinds::Type::Ability1;
	GenericCancelInputID = EAbilityInputBinds::Type::Ability8;
}

FActiveGameplayEffectHandle UURAbilitySystemComponent::ApplyGameplayEffectSpecToSelf(const FGameplayEffectSpec& GameplayEffect, FPredictionKey PredictionKey)
{
	// 죽었을 땐 GE 적용 X
	if (IsDead())
	{
		return FActiveGameplayEffectHandle();
	}
	return Super::ApplyGameplayEffectSpecToSelf(GameplayEffect, PredictionKey);
}

void UURAbilitySystemComponent::NotifyAbilityFailed(
	const FGameplayAbilitySpecHandle Handle, 
	UGameplayAbility* Ability,
	const FGameplayTagContainer& FailureReason)
{
	// Super::NotifyAbilityFailed(Handle, Ability, FailureReason);
	const int16 PredictionKey = AbilityActorInfo->IsLocallyControlled() ? 0 : ActivatingPredictionKeyOnServer;
	NotifyAbilityFailed(Handle, PredictionKey, Ability, FailureReason);
}

void UURAbilitySystemComponent::AbilityInputTagTriggered(const EAbilityInputBinds::Type& InputBinding)
{
	const int32 InputID = InputBinding;

	if (ConsumedInputIDs.Contains(InputID))
	{
		// Confirm/Cancel 되어 소모된 InputID는 Released 전까지 사용하지 Trigger되어도 무시한다.
		return;
	}

	// Consume the input if this InputID is overloaded with GenericConfirm/Cancel and the GenericConfim/Cancel callback is bound
	/*
	 * Note: LocalInputConfirm에서 GenericLocalConfirmCallbacks를 비우는데, 
	 * TargetActor에서는 Confirm할 때 IsConfirmTargetingAllowed를 검사하고 false일 경우엔 완료 시키지 않음.
	 * 즉 한 번 실패하면 다시 Confirm 할 수 없음
	 * TargetConfirm에선 IsConfirmTargetingAllowed가 false일 경우 다시 등록하므로 해결 가능하나, 
	 * 타겟 액터가 아닌 확인/취소 기능이 있는 WaitConfirmCancel 등을 사용할 경우 수정 필요함. 
	 */
	const bool bHasSpawnedTargetActor = SpawnedTargetActors.ContainsByPredicate([](TObjectPtr<AGameplayAbilityTargetActor> TargetActor)
	{
		return IsValid(TargetActor);
	});
	if (bHasSpawnedTargetActor)
	{
		if (InputID == GenericConfirmInputID)
		{
			UE_LOG(LogTemp, Warning, TEXT("GenericConfirmInputID"));
			ConsumedInputIDs.Add(InputID);
			TargetConfirm();
			return;
		}
		
		if (InputID == GenericCancelInputID)
		{
			ConsumedInputIDs.Add(InputID);
			TargetCancel();
			return;
		}
	}
	// if (IsGenericConfirmInputBound(InputID))
	// {
	// 	ConsumedInputIDs.Add(InputID);
	// 	LocalInputConfirm();
	// 	return;
	// }
	//
	// if (IsGenericCancelInputBound(InputID))
	// {
	// 	ConsumedInputIDs.Add(InputID);
	// 	LocalInputCancel();
	// 	return;
	// }

	const FGameplayAbilitySpecHandle CurrentAbilityHandle = ResolveCurrentAbilityForInput(InputBinding);
	FGameplayAbilitySpec* CurrentAbilitySpec = FindAbilitySpecFromHandle(CurrentAbilityHandle);
	
	if (CurrentAbilitySpec && CurrentAbilitySpec->Ability)
	{
		CurrentAbilitySpec->InputPressed = true;
		if (CurrentAbilitySpec->IsActive())
		{
			if (CurrentAbilitySpec->Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
			{
				ServerSetInputPressed(CurrentAbilityHandle);
			}

			AbilitySpecInputPressed(*CurrentAbilitySpec);

			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			// Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
			UE_CLOG(
				CurrentAbilitySpec->Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution,
				LogAbilitySystem, Warning,
				TEXT(
					"%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"
				), __func__, *GetNameSafe(CurrentAbilitySpec->Ability));
			TArray<UGameplayAbility*> Instances = CurrentAbilitySpec->GetAbilityInstances();
			const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty()
				                                                       ? CurrentAbilitySpec->ActivationInfo
				                                                       : Instances.Last()->
				                                                       GetCurrentActivationInfoRef();
			PRAGMA_ENABLE_DEPRECATION_WARNINGS
			// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, CurrentAbilityHandle,
			                      ActivationInfo.GetActivationPredictionKey());
		}
		else
		{
			if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(AbilityActorInfo->PlayerController))
			{
				GamePC->TryActivateAbility(CurrentAbilityHandle);
			}
			else
			{
				TryActivateAbility(CurrentAbilityHandle);
			}
		}
	}
}

void UURAbilitySystemComponent::AbilityInputTagReleased(const EAbilityInputBinds::Type& InputBinding)
{
	if (ConsumedInputIDs.Remove(InputBinding) > 0)
	{
		// Confirm/Cancel 되어 소모된 InputID는 Released 이벤트를 무시함
		return;
	}
	AbilityLocalInputReleased(InputBinding);
}

void UURAbilitySystemComponent::ClearAbilityInput()
{
	// UAbilitySystemComponent::AbilityLocalInputReleased에서 InputID 검사 부분을 제거하고 InputPressed 조건을 추가함
	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputPressed)
		{
			Spec.InputPressed = false;
			if (Spec.Ability && Spec.IsActive())
			{
				if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
				{
					ServerSetInputReleased(Spec.Handle);
				}

				AbilitySpecInputReleased(Spec);

PRAGMA_DISABLE_DEPRECATION_WARNINGS
				// Fixing this up to use the instance activation, but this function should be deprecated as it cannot work with InstancedPerExecution
				UE_CLOG(Spec.Ability->GetInstancingPolicy() == EGameplayAbilityInstancingPolicy::InstancedPerExecution, LogAbilitySystem, Warning, TEXT("%hs: %s is InstancedPerExecution. This is unreliable for Input as you may only interact with the latest spawned Instance"), __func__, *GetNameSafe(Spec.Ability));
				TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
				const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty() ? Spec.ActivationInfo : Instances.Last()->GetCurrentActivationInfoRef();
PRAGMA_ENABLE_DEPRECATION_WARNINGS
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
	
	ConsumedInputIDs.Empty();
}

// ~Begin Tag Relationship Mapping
void UURAbilitySystemComponent::SetTagRelationshipMapping(UURAbilityTagRelationshipMapping* NewMapping)
{
	TagRelationshipMapping = NewMapping;
}

void UURAbilitySystemComponent::GetAdditionalActivationTagRequirements(
	const FGameplayTagContainer& AbilityTags,
	FGameplayTagContainer& OutActivationRequired, 
	FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMapping)
	{
		TagRelationshipMapping->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}

void UURAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(
	const FGameplayTagContainer& AbilityTags,
	UGameplayAbility* RequestingAbility,
	bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags,
	bool bExecuteCancelTags,
	const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMapping)
	{
		// Use the mapping to expand the ability tags into block and cancel tag
		TagRelationshipMapping->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: Apply any special logic like blocking input or movement
}

// ~End Tag Relationship Mapping


FGameplayAbilitySpecHandle UURAbilitySystemComponent::ResolveCurrentAbilityForInput(const EAbilityInputBinds::Type& InputBinding) const
{
	TArray<const FGameplayAbilitySpec*> Specs;
	FindAllAbilitySpecsFromInputID(InputBinding, Specs);
	
	if (!Specs.IsEmpty())
	{
		// 사용 조건 충족 여부와 상관없이 최우선 어빌리티
		int IndexOfFallbackAbility = -1;
		int FallbackPriority = MAX_int32;
		// 사용 조건을 충족한 어빌리티 중 최우선 어빌리티
		int IndexOfSelectedAbility = -1;
		int SelectedPriority = MAX_int32;

		for (int i = 0; i < Specs.Num(); i++)
		{
			UGameplayAbility* Ability = Specs[i]->Ability;
		
			if (!Ability)
			{
				continue;
			}
		
			UGameplayAbility* InstancedAbility = Specs[i]->GetPrimaryInstance();
			UGameplayAbility* AbilitySource = InstancedAbility ? InstancedAbility : Ability;

			if (const UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(AbilitySource))
			{
				const int Priority = ActiveAbility->GetPriority();
				
				// Fallback 설정
				if (IndexOfFallbackAbility < 0 || 
					Priority < FallbackPriority)
				{
					IndexOfFallbackAbility = i;
					FallbackPriority = Priority;
				}
				
				// 사용 조건 충족 체크 후 지정
				if (IndexOfSelectedAbility < 0 || 
					Priority < SelectedPriority)
				{
					if (ActiveAbility->DoesAbilitySatisfyTagRequirementsToSelect(*this, &GetOwnedGameplayTags()))
					{
						IndexOfSelectedAbility = i;
						SelectedPriority = Priority;
					}
				}
			}
			else
			{
				// Fallback 설정
				if (IndexOfFallbackAbility < 0)
				{
					IndexOfFallbackAbility = i;
				}
			}
		}
	
		if (Specs.IsValidIndex(IndexOfSelectedAbility))
		{
			return Specs[IndexOfSelectedAbility]->Handle;
		}

		// 사용 조건 충족한 어빌리티 없을 경우 fallback 어빌리티 핸들 리턴
		if (Specs.IsValidIndex(IndexOfFallbackAbility))
		{
			return Specs[IndexOfFallbackAbility]->Handle;
		}
	}
	
	return FGameplayAbilitySpecHandle();
}

void UURAbilitySystemComponent::Respawn()
{
	if (!GetOwnerActor() || !GetOwnerActor()->HasAuthority())
	{
		return;
	}
	
	SetNumericAttributeBase(UURHealthSet::GetHealthAttribute(), GetNumericAttribute(UURHealthSet::GetMaxHealthAttribute()));
}

void UURAbilitySystemComponent::NotifyAbilityFailed(
	const FGameplayAbilitySpecHandle Handle, 
	const int16 PredictionKey,
	UGameplayAbility* Ability, 
	const FGameplayTagContainer& FailureReason)
{
	APlayerController* PC = AbilityActorInfo->PlayerController.Get();
	const bool bIsLocallyControlled = AbilityActorInfo->IsLocallyControlled();
	if (IsOwnerActorAuthoritative() &&
		!bIsLocallyControlled)
	{
		// 서버에선 클라이언트에 실패 RPC 전송
		if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(PC))
		{
			GamePC->ClientAbilityFailed(Handle, PredictionKey, FailureReason);
		}
		// 서버에서 등록한 AbilityFailedCallback이 있을 수 있기 때문에 return 하지 않고 Super:: 호출한다.
	}
	
	if (bIsLocallyControlled)
	{
		// AbilityFailedCallbacks에 등록해 호출될 경우 PredictionKey가 전달되지 않으므로 직접 호출
		if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(PC))
		{
			GamePC->LocalAbilityFailed(Handle, PredictionKey, FailureReason);
		}
	}
	
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);
}

FGameplayEffectContextHandle UURAbilitySystemComponent::MakeDamageEffectContext() const
{
	FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(new FURHitGameplayEffectContext());
	
	// By default use the owner and avatar as the instigator and causer
	if (ensureMsgf(AbilityActorInfo.IsValid(), TEXT("Unable to make effect context because AbilityActorInfo is not valid.")))
	{
		Context.AddInstigator(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get());
	}
	
	return Context;
}

void UURAbilitySystemComponent::InternalServerTryActivateAbility(
	FGameplayAbilitySpecHandle AbilityToActivate,
	bool InputPressed, 
	const FPredictionKey& PredictionKey, 
	const FGameplayEventData* TriggerEventData)
{
	// 서버에서 TryActivate 할 때 PredictionKey를 임시 저장해 실패할 경우 NotifyAbilityFailed에 PredictionKey를 전달할 수 있도록 함.
	ActivatingPredictionKeyOnServer = PredictionKey.Current;

	ABILITYLIST_SCOPE_LOCK();
	
	// TriggerEventData의 값을 서버 기준의 값으로 재설정
	FGameplayEventData ServerEventData;
	const FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(AbilityToActivate);
	const UURGameplayAbility_ActiveAbility* ActiveAbility = Spec ? Cast<UURGameplayAbility_ActiveAbility>(Spec->Ability) : nullptr;
	if (ActiveAbility)
	{
		if (!TriggerEventData && 
			ActiveAbility->GetTargetingPolicy() == EAbilityTargetingPolicy::Type::Targeting)
		{
			// 타겟 데이터가 있어야만 함. 
			ClientActivateAbilityFailed(AbilityToActivate, PredictionKey.Current);
			return;
		}
		
		if (TriggerEventData)
		{
			ServerEventData.Instigator = GetOwnerActor();
			ServerEventData.InstigatorTags = GetOwnedGameplayTags();
			const AActor* Target = TriggerEventData ? TriggerEventData->Target.Get() : nullptr;
			ServerEventData.Target = Target;
			if (IsValid(Target))
			{
				if (const UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target))
				{
					ServerEventData.TargetTags = TargetASC->GetOwnedGameplayTags();
				}
			}
			TriggerEventData = &ServerEventData;
		}
	}
	
	Super::InternalServerTryActivateAbility(AbilityToActivate, InputPressed, PredictionKey, TriggerEventData);
	ActivatingPredictionKeyOnServer = 0;
}

void UURAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	
	// Note: OnRep_ActivateAbilities에서도 호출 가능하도록 파라미터 제거하고 액티브 어빌리티가 아닌 어빌리티 변경에 따른 델리게이트로 변경함.
	// if (AbilitySpec.Ability && AbilitySpec.Ability->IsA(UURGameplayAbility_ActiveAbility::StaticClass()))
	// {
	// 	OnActiveAbilityChanged.Broadcast(AbilitySpec);
	// }
	QueueAbilityListChanged();
}

void UURAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);
	
	// Note: OnRep_ActivateAbilities에서도 호출 가능하도록 파라미터 제거하고 액티브 어빌리티가 아닌 어빌리티 변경에 따른 델리게이트로 변경함.
	// if (AbilitySpec.Ability && AbilitySpec.Ability->IsA(UURGameplayAbility_ActiveAbility::StaticClass()))
	// {
	// 	OnActiveAbilityChanged.Broadcast(AbilitySpec);
	// }
	QueueAbilityListChanged();
}

void UURAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	
	QueueAbilityListChanged();
}

void UURAbilitySystemComponent::QueueAbilityListChanged()
{
	if (AbilityListChangedTimer.IsValid())
	{
		return;
	}

	AbilityListChangedTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::BroadcastAbilityListChanged);
}

void UURAbilitySystemComponent::BroadcastAbilityListChanged()
{
	AbilityListChangedTimer.Invalidate();
	OnAbilityListChanged.Broadcast();
}
