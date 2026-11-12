// Written by EXPx2


#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "URGame.h"
#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "AbilitySystem/AbilityTask/AbilityTask_AbilityActionTimeline.h"
#include "AbilitySystem/AbilityTask/AbilityTask_LookTarget.h"
#include "AbilitySystem/AbilityTask/AbilityTask_SimpleWaitTargetData.h"
#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_Custom.h"
#include "AbilitySystem/AttributeSet/URPlayerResourceSet.h"
#include "AbilitySystem/GameplayEffect/URHitGameplayEffectContext.h"
#include "AbilitySystem/GameplayEffect/GameplayEffect_GrantTag.h"
#include "AbilitySystem/GameplayEffect/GameplayEffect_Cooldown.h"
#include "AbilitySystem/GameplayEffect/GameplayEffect_ManaCost.h"
#include "AbilitySystem/TargetData/GameplayAbilityTargetData_Time.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Character/URCombatComponent.h"
#include "Character/URTargetingComponent.h"
#include "BlueprintLibrary/URCombatBlueprintLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTags/URGameplayTags.h"
#include "Interface/URDamageableInterface.h"
#include "Interface/URTargetableInterface.h"
#include "Player/URGamePlayerController.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "GameplayAbility"

UURGameplayAbility_ActiveAbility::UURGameplayAbility_ActiveAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	static ConstructorHelpers::FClassFinder<UGameplayEffect> InCombatGERef(TEXT("/Game/_ProjectUR/GAS/GameplayEffects/BPGE_InCombat"));
	InCombatGE = InCombatGERef.Class;
	
	CostGameplayEffectClass = UGameplayEffect_ManaCost::StaticClass();
	CooldownGameplayEffectClass = UGameplayEffect_Cooldown::StaticClass();
	
	static ConstructorHelpers::FClassFinder<AGameplayAbilityTargetActor_GroundTrace> GroundTraceClassRef(TEXT("/Game/_ProjectUR/GAS/TargetActor/BPTA_GroundTrace"));
	GroundTraceClass = GroundTraceClassRef.Class;

	static ConstructorHelpers::FClassFinder<AActor> ReticleClassRef(TEXT("/Game/_ProjectUR/Misc/BP_AOEReticle"));
	ReticleClass = ReticleClassRef.Class;
}

bool UURGameplayAbility_ActiveAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	/*
	 * Note: URGameplayAbility_Ability는 반드시 타겟을 포함(nullptr 허용)한 FGameplayEventData를 통해 Trigger되어야만 활성화 가능하다.
	 * UAbilitySystemComponent::InternalTryActivateAbility에서 EventData가 없을 경우에 nullptr로 설정한다
	 * const FGameplayTagContainer* SourceTags = TriggerEventData ? &TriggerEventData->InstigatorTags : nullptr;
	 * SourceTags가 nullptr일 경우 EventData가 없이 호출된 상황으로 리턴함
	 * 타겟이 지정되기 전에 가능성을 검사하기 위해 TargetTags == nullptr일 경우엔 통과시킴
	 * 활성화가 아닌 활성화 가능성을 얻기 위해 함수를 호출할 때 Tags가 없어 잘못된 결과가 나올 가능성 있으니 SourceTags에 빈 TagContainer라도 전달해야함.
	 */
	if (SourceTags == nullptr)// || TargetTags == nullptr)
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(URGameplayTags::TAG_Ability_Fail_BadRequest);
		}
		return false;
	}
	
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

bool UURGameplayAbility_ActiveAbility::ShouldAbilityRespondToEvent(
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayEventData* Payload) const
{
	/*
	 * Note: Activate 되기 전에 타겟을 검사하여 Activate 자체를 막을 수 있지만, FailureReason을 사용할 수 없어 Activate 허용함.
	 */
	// if (!Super::ShouldAbilityRespondToEvent(ActorInfo, Payload))
	// {
	// 	return false;
	// }
	//
	// return CanActivateWithTarget(Payload->Target.Get(), ActorInfo, nullptr);
	
	return Super::ShouldAbilityRespondToEvent(ActorInfo, Payload);
}

const FGameplayTagContainer* UURGameplayAbility_ActiveAbility::GetCooldownTags() const
{
	UGameplayEffect* CDGE = GetCooldownGameplayEffect();
	if (CDGE)
	{
		if (CDGE->IsA<UGameplayEffect_Cooldown>())
		{
			CachedCooldownTags.Reset();
			if (CooldownTag.IsValid())
			{
				CachedCooldownTags.AddTag(CooldownTag);
			}
			return &CachedCooldownTags;
		}
		return &CDGE->GetGrantedTags();
	}

	return nullptr;
}

bool UURGameplayAbility_ActiveAbility::CommitCheck(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags)
{
	bool bResult = Super::CommitCheck(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags);
	
	if (bResult)
	{
		// ConsumeTag 체크
		if (!CheckConsumeTag(Handle, ActorInfo, OptionalRelevantTags))
		{
			return false;
		}
	}
	
	return bResult;
}

void UURGameplayAbility_ActiveAbility::CommitExecute(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);

	ApplyConsumeTags(Handle,ActorInfo,ActivationInfo);
	GrantInCombatGameplayEffect(Handle, ActorInfo, ActivationInfo);
	GrantFollowupTag(Handle, ActorInfo, ActivationInfo);
}

void UURGameplayAbility_ActiveAbility::ApplyCooldown(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CooldownGE = GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		if (CooldownGE->IsA<UGameplayEffect_Cooldown>())
		{
			// 쿨다운 시간 및 태그 적용
			if (Cooldown > SMALL_NUMBER &&
				CooldownTag.IsValid())
			{
				const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(
					Handle,
					ActorInfo,
					ActivationInfo,
					CooldownGE->GetClass(),
					GetAbilityLevel(Handle, ActorInfo));

				if (Spec.IsValid())
				{
					// 쿨타임 시간 적용
					Spec.Data->SetSetByCallerMagnitude(URGameplayTags::SetByCaller_Duration, Cooldown);
					// 쿨타임 태그 적용
					Spec.Data->DynamicGrantedTags.AddTag(CooldownTag);
					ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
				}
			}
			return;
		}
		else
		{
			// Super::ApplyCooldown
			ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, CooldownGE, GetAbilityLevel(Handle, ActorInfo));
		}
	}
}

bool UURGameplayAbility_ActiveAbility::CheckCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	UGameplayEffect* CostGE = GetCostGameplayEffect();
	if (CostGE)
	{
		UAbilitySystemComponent* AbilitySystemComponent = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
		if (ensure(AbilitySystemComponent))
		{
			const bool bCanApplyCost = CostGE->IsA<UGameplayEffect_ManaCost>()
				? AbilitySystemComponent->GetNumericAttribute(UURPlayerResourceSet::GetManaAttribute()) >= ManaCost
				: AbilitySystemComponent->CanApplyAttributeModifiers(CostGE, GetAbilityLevel(Handle, ActorInfo), MakeEffectContext(Handle, ActorInfo));
			if (!bCanApplyCost)
			{
				const FGameplayTag& CostTag = UAbilitySystemGlobals::Get().ActivateFailCostTag;

				if (OptionalRelevantTags && CostTag.IsValid())
				{
					OptionalRelevantTags->AddTag(CostTag);
				}
				return false;
			}
		}
	}
	return true;
}

void UURGameplayAbility_ActiveAbility::ApplyCost(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UGameplayEffect* CostGE = GetCostGameplayEffect();
	if (CostGE)
	{
		if (CostGE->IsA<UGameplayEffect_ManaCost>())
		{
			// 마나 소모값 설정
			const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(
				Handle,
				ActorInfo,
				ActivationInfo,
				CostGE->GetClass(),
				GetAbilityLevel(Handle, ActorInfo));

			if (Spec.IsValid())
			{
				// 소모값 설정
				Spec.Data->SetSetByCallerMagnitude(URGameplayTags::SetByCaller_ManaCost, -ManaCost);
				ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
			}
			return;
		}

		// Suer::ApplyCost
		ApplyGameplayEffectToOwner(Handle, ActorInfo, ActivationInfo, CostGE, GetAbilityLevel(Handle, ActorInfo));
	}
}

#if WITH_EDITOR
EDataValidationResult UURGameplayAbility_ActiveAbility::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (int i = 0; i < Actions.Num(); ++i)
	{
		const FURAbilityActionBase* Action = Actions[i].GetPtr();
		if (!Action)
		{
			Context.AddError(FText::Format(LOCTEXT("MissingAction", "{0} 액션이 None입니다."), i));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (!Action->HasValidData())
		{
			Context.AddError(FText::Format(LOCTEXT("InvalidAction", "{0}({1}) 액션의 설정이 잘못되었습니다."), Actions[i].GetScriptStruct()->GetDisplayNameText(), i));
			Result = EDataValidationResult::Invalid;
		}
	}
	return Result;
}

#endif

bool UURGameplayAbility_ActiveAbility::CanActivateAbilityToTarget(
	const UAbilitySystemComponent* AbilitySystemComponent, 
	const FGameplayAbilitySpecHandle Handle,
	AActor* Target,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!IsValid(AbilitySystemComponent) || !Handle.IsValid())
	{
		if (OptionalRelevantTags)
		{
			OptionalRelevantTags->AddTag(URGameplayTags::TAG_Ability_Fail_BadRequest);
		}
		return false;
	}
	
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	return CanActivateAbility(
		Handle,
		AbilitySystemComponent->AbilityActorInfo.Get(),
		&AbilitySystemComponent->GetOwnedGameplayTags(),
		TargetASC ? &TargetASC->GetOwnedGameplayTags() : nullptr,
		OptionalRelevantTags);
}

bool UURGameplayAbility_ActiveAbility::K2_CanActivateAbilityToTarget(
	const UAbilitySystemComponent* AbilitySystemComponent, 
	const FGameplayAbilitySpecHandle Handle,
	AActor* Target) const
{
	FGameplayTagContainer Failure;
	return CanActivateAbilityToTarget(AbilitySystemComponent, Handle, Target, &Failure);
}

void UURGameplayAbility_ActiveAbility::GetActivationTagRequirements(
	const UAbilitySystemComponent& AbilitySystemComponent,
	FGameplayTagContainer& OutRequiredTags,
	FGameplayTagContainer& OutBlockedTags) const
{
	Super::GetActivationTagRequirements(AbilitySystemComponent, OutRequiredTags, OutBlockedTags);

	if (!bCanActivateWhileFalling)
	{
		OutBlockedTags.AddTag(URGameplayTags::Character_MovementMode_Falling);
	}
}

bool UURGameplayAbility_ActiveAbility::DoesAbilitySatisfyTagRequirementsToSelect(
	const UAbilitySystemComponent& AbilitySystemComponent,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags) const
{
	// DoesAbilitySatisfyTagRequirements 와 같으나 OptionalRelevantTags 설정과 Relationship 태그를 제외함

	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	// Check if any of this ability's tags are currently blocked
	// if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	// {
	// 	bBlocked = true;
	// }

	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	if (const FGameplayTagContainer* CooldownTags = GetCooldownTags())
	{
		AllBlockedTags.AppendTags(*CooldownTags);
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	// Todo: 사용 가능 범위안에 태그를 가진 적 있을 경우 표시
	// 예) 전투 타겟이나 표시 타겟이 속박 태그가 없어도 사용 가능 범위 안에 속박 태그를 가진 적이 있을 경우 표시
	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		return false;
	}
	if (bMissing)
	{
		return false;
	}

	return true;
}

bool UURGameplayAbility_ActiveAbility::CanDisplayAsEnabled(
	const UAbilitySystemComponent& AbilitySystemComponent,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags) const
{
	// DoesAbilitySatisfyTagRequirements 와 같으나 OptionalRelevantTags 설정과 Relationship 태그를 제외함

	// Specialized version to handle death exclusion and AbilityTags expansion via ASC

	bool bBlocked = false;
	bool bMissing = false;

	// Check if any of this ability's tags are currently blocked
	// if (AbilitySystemComponent.AreAbilityTagsBlocked(GetAssetTags()))
	// {
	// 	bBlocked = true;
	// }

	static FGameplayTagContainer AllRequiredTags;
	static FGameplayTagContainer AllBlockedTags;

	AllRequiredTags = ActivationRequiredTags;
	AllBlockedTags = ActivationBlockedTags;

	if (!UAbilitySystemGlobals::Get().ShouldIgnoreCosts() && !CheckCost(CurrentSpecHandle, CurrentActorInfo))
	{
		// == K2_CheckAbilityCost
		bBlocked = true;
	}

	// Check to see the required/blocked tags for this ability
	if (AllBlockedTags.Num() || AllRequiredTags.Num())
	{
		static FGameplayTagContainer AbilitySystemComponentTags;

		AbilitySystemComponentTags.Reset();
		AbilitySystemComponent.GetOwnedGameplayTags(AbilitySystemComponentTags);

		if (AbilitySystemComponentTags.HasAny(AllBlockedTags))
		{
			bBlocked = true;
		}

		if (!AbilitySystemComponentTags.HasAll(AllRequiredTags))
		{
			bMissing = true;
		}
	}

	if (SourceTags != nullptr)
	{
		if (SourceBlockedTags.Num() || SourceRequiredTags.Num())
		{
			if (SourceTags->HasAny(SourceBlockedTags))
			{
				bBlocked = true;
			}

			if (!SourceTags->HasAll(SourceRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	// Todo: 사용 가능 범위안에 태그를 가진 적 있을 경우 표시
	// 예) 전투 타겟이나 표시 타겟이 속박 태그가 없어도 사용 가능 범위 안에 속박 태그를 가진 적이 있을 경우 표시
	if (TargetTags != nullptr)
	{
		if (TargetBlockedTags.Num() || TargetRequiredTags.Num())
		{
			if (TargetTags->HasAny(TargetBlockedTags))
			{
				bBlocked = true;
			}

			if (!TargetTags->HasAll(TargetRequiredTags))
			{
				bMissing = true;
			}
		}
	}

	if (bBlocked)
	{
		return false;
	}
	if (bMissing)
	{
		return false;
	}

	return true;
}

bool UURGameplayAbility_ActiveAbility::IsValidTarget(const AActor* SourceActor, const AActor* Actor) const
{
	return UURGameBlueprintLibrary::IsDamageableActor(Actor) &&
		EnumHasAnyFlags(UURCombatBlueprintLibrary::GetRelativeTargetFaction(GetTargetFactions(), SourceActor), IURDamageableInterface::Execute_GetFaction(Actor));
}

FGameplayEffectContextHandle UURGameplayAbility_ActiveAbility::MakeHitEffectContext(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	// UGameplayAbility::MakeEffectContext
	FGameplayEffectContextHandle Context = FGameplayEffectContextHandle(new FURHitGameplayEffectContext());
	Context.SetAbility(this);

	if (ensure(ActorInfo))
	{
		// By default use the owner and avatar as the instigator and causer
		Context.AddInstigator(ActorInfo->OwnerActor.Get(), ActorInfo->AvatarActor.Get());

		// Pass along the source object to the effect
		if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			if (FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(Handle))
			{
				Context.AddSourceObject(AbilitySpec->SourceObject.Get());
			}
		}
	}

	return Context;
	
}

void UURGameplayAbility_ActiveAbility::ActivateCustomAbilityAction_Implementation(
	UAbilityActionTask_Custom* CustomTask, 
	FURAbilityActionContext InContext, 
	const float ElapsedTime)
{
	CustomTask->FinishTask(true);
}

float UURGameplayAbility_ActiveAbility::GetUseRange(const bool bIsForRemoteClient) const
{
	// 서버에서는 지연과 오차에 대응해 조금 더 먼 거리를 허용한다.
	// Note: 핑이나 캐릭터의 이동 시간에 따라 값을 조절하도록 개선 가능
	if (bIsForRemoteClient)
	{
		return UseRange + FMath::Clamp(UseRange * 0.1f, 30.f, 100.f);
	}

	return UseRange;
}

void UURGameplayAbility_ActiveAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (TriggerEventData == nullptr)
	{
		EndAbilityAsFailed(URGameplayTags::TAG_Ability_Fail_Undefined);
		return;
	}
	
	const AActor* Target = TriggerEventData->Target.Get();
	FGameplayTagContainer FailureReason;
	
	// 통과하여 활성화되더라도 조건이 변경되었으면 활성화 취소
	if (!CanActivateWithTarget(Target, ActorInfo, &FailureReason))
	{
		EndAbilityAsFailed(FailureReason);
		return;
	}

	// 초기화
	Cleanup();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AbilityTargetPtr = Target ? const_cast<AActor*>(Target) : nullptr;
	
	if (GetTargetingPolicy() == EAbilityTargetingPolicy::Range)
	{
		check(GroundTraceClass);

		UAbilityTask_WaitTargetData* WaitTargetData =
			UAbilityTask_WaitTargetData::WaitTargetData(
				this,
				NAME_None,
				EGameplayTargetingConfirmation::UserConfirmed,
				GroundTraceClass.Get());

		WaitTargetData->ValidData.AddUniqueDynamic(this, &ThisClass::ValidateRangeTargetData);
		WaitTargetData->Cancelled.AddUniqueDynamic(this, &ThisClass::HandleTargetDataCancelled);
		WaitTargetData->ReadyForActivation();

		if (WaitTargetData->IsFinished())
		{
			return;
		}

		AGameplayAbilityTargetActor* SpawnedActor = nullptr;
		if (WaitTargetData->BeginSpawningActor(this, GroundTraceClass.Get(), SpawnedActor))
		{
			AGameplayAbilityTargetActor_GroundTrace* TargetActor = CastChecked<AGameplayAbilityTargetActor_GroundTrace>(SpawnedActor);
			TargetActor->SetOwner(GetOwningActorFromActorInfo());
			TargetActor->MaxRange = UseRange;
			TargetActor->TraceProfile = FCollisionProfileName("GroundTrace");
			TargetActor->bTraceAffectsAimPitch = true;
			TargetActor->StartLocation = MakeTargetLocationInfoFromOwnerActor();
			if (IsValid(ReticleClass))
			{
				TargetActor->ReticleClass = ReticleClass;
				TargetActor->ReticleParams = FWorldReticleParameters(FVector(ReticleScale));
			}

			WaitTargetData->FinishSpawningActor(this, TargetActor);
		}
		return;
	}
	
	TargetLocation = Target && TargetingPolicy != EAbilityTargetingPolicy::NonTargeting
		                 ? UURGameBlueprintLibrary::GetFeetLocation(Target)
		                 : UURGameBlueprintLibrary::GetFeetLocation(GetAvatarActorFromActorInfo());
	
	ActionAbility(Handle, ActorInfo, ActivationInfo);
}

void UURGameplayAbility_ActiveAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 초기화
	Cleanup();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	OnAbilityEnded.Broadcast();
	
	if (!bWasCancelled)
	{
		// GrantFollowupTag(Handle, ActorInfo,ActivationInfo);
	}
}

void UURGameplayAbility_ActiveAbility::EndAbilityAsFailed(const FGameplayTagContainer& FailureReason)
{
	if (UURAbilitySystemComponent* URASC = Cast<UURAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		URASC->NotifyAbilityFailed(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey().Current, this, FailureReason);
	}

	// if (IsForRemoteClient())
	// {
	// 	// 어빌리티 실패 RPC 전송
	// 	if (AURPlayerController* URPlayerController = Cast<AURPlayerController>(GetActorInfo().PlayerController.Get()))
	// 	{
	// 		URPlayerController->ClientAbilityFailed(GetCurrentAbilitySpecHandle(), GetCurrentActivationInfo().GetActivationPredictionKey(), FailureReason);
	// 	}
	// }
	// else
	// {
	// 	if (UURAbilitySystemComponent* URASC = Cast<UURAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	// 	{
	// 		URASC->NotifyAbilityFailed(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey().Current, this, FailureReason);
	// 	}
	// }
	bool bReplicateEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UURGameplayAbility_ActiveAbility::Cleanup()
{
	AbilityTargetPtr.Reset();
	
	if (ActiveDurationTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ActiveDurationTimerHandle);
	}
		
	bIsActiveDurationOver = false;
	bIsTimelineCompleted = false;
}

void UURGameplayAbility_ActiveAbility::ValidateRangeTargetData(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!ensure(GetTargetingPolicy() == EAbilityTargetingPolicy::Range))
	{
		// Range 타입이 아닐 경우 호출되지 않아야 함.
		EndAbilityAsFailed(URGameplayTags::TAG_Ability_Fail_Undefined);
		return;
	}
	
	if (!bCanActivateWhileFalling)
	{
		// 공중 사용 불가 체크
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			if (ASC->GetOwnedGameplayTags().HasTag(URGameplayTags::Character_MovementMode_Falling))
			{
				EndAbilityAsFailed(URGameplayTags::TAG_Ability_Fail_CanActivateAbilityTag);
				return;
			}
		}
	}

	if (const FGameplayAbilityTargetData* TargetData = Data.Get(0))
	{
		if (TargetData->HasHitResult())
		{
			// 검증
			const FVector HitLocation = TargetData->GetHitResult()->Location;
			if (const AActor* AvatarActor = GetAvatarActorFromActorInfo())
			{
				const FVector AvatarLocation = UURGameBlueprintLibrary::GetFeetLocation(AvatarActor);
				const float SqrDistance = FVector::DistSquared(HitLocation, AvatarLocation);
				const float DistanceThreshold = GetUseRange(IsForRemoteClient()) + 50.f; // Tolerance
				if (SqrDistance <= DistanceThreshold * DistanceThreshold)
				{
					TargetLocation = HitLocation;
					ActionAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
					return;
				}

				// 거리 밖임
				EndAbilityAsFailed(URGameplayTags::TAG_Ability_Fail_InvalidTarget);
				return;
			}

			// 아바타 액터 없음
			EndAbilityAsFailed(URGameplayTags::TAG_Ability_Fail_Undefined);
			return;
		}
	}

	EndAbilityAsFailed(URGameplayTags::TAG_Ability_Fail_NoTarget);
}

void UURGameplayAbility_ActiveAbility::HandleTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	EndAbilityAsFailed();
}

bool UURGameplayAbility_ActiveAbility::CanActivateWithTarget(
	const AActor* TargetActor, 
	const FGameplayAbilityActorInfo* ActorInfo, 
	OUT FGameplayTagContainer* FailureReason) const
{
	if (TargetingPolicy != EAbilityTargetingPolicy::Targeting && 
	TargetingPolicy != EAbilityTargetingPolicy::NonTargetingWithTarget)
	{
		// 타겟 유무 상관 없을 경우 즉시 리턴
		return true;
	}
	
	if (TargetActor == nullptr)
	{
		// 타겟이 없을 경우
		if (TargetingPolicy == EAbilityTargetingPolicy::Targeting)
		{
			// 타겟이 있어야만 하는 스킬은 실패
			if (FailureReason)
			{
				FailureReason->AddTag(URGameplayTags::TAG_Ability_Fail_NoTarget);
			}
			return false;
		}
		
		return true;
	}
	
	// 타겟이 있을 경우
	if (!IsValidTarget(ActorInfo->OwnerActor.Get(), TargetActor))
	{
		// 유효하지 않은 타겟
		if (FailureReason)
		{
			FailureReason->AddTag(URGameplayTags::TAG_Ability_Fail_InvalidTarget);
		}
		return false;
	}
	
	// 거리 검사
	const bool bIsLocallyControlled = ActorInfo->IsLocallyControlled();
	const bool bIsAuthority = ActorInfo->IsNetAuthority();
	const bool bIsForRemoteClient = !bIsLocallyControlled && bIsAuthority;
	const AActor* AvatarActor = ActorInfo->AvatarActor.Get();

	const UPathFollowingComponent* PathFollowingComp = nullptr;
	if (APlayerController* PC = GetActorInfo().PlayerController.Get())
	{
		PathFollowingComp = UURGameBlueprintLibrary::GetPathFollowingComponent(PC, false);
	}
	else if (const APawn* Pawn = Cast<APawn>(AvatarActor))
	{
		PathFollowingComp = UURGameBlueprintLibrary::GetPathFollowingComponent(Pawn->Controller, false);
	}

	if (PathFollowingComp)
	{
		// UPathFollowingComponent가 있을 경우 UPathFollowingComponent::HasReached 기준으로 검사
		if (UURGameBlueprintLibrary::IsActorInRangeUsingPathFollowingComponent(PathFollowingComp, TargetActor, GetUseRange(bIsForRemoteClient)))
		{
			return true;
		}
		
		if (FailureReason)
		{
			FailureReason->AddTag(URGameplayTags::TAG_Ability_Fail_PathFollowingFailed);
		}
		return false;
	}
	// UPathFollowingComponent가 없을 경우 단순 거리 검사
	
	// 액터의 높이가 다를 경우 같은 거리여도 결과가 바뀔 수 있기 때문에 FeetLocation 사용
	const float SqrDistance = FVector::DistSquared(UURGameBlueprintLibrary::GetFeetLocation(AvatarActor), UURGameBlueprintLibrary::GetFeetLocation(TargetActor));
	// return FVector::DistSquared(AvatarActor->GetActorLocation(), TargetActor->GetActorLocation()) >
	// 	FMath::Square(GetUseRange(bIsForRemoteClient));
	if (SqrDistance <= FMath::Square(GetUseRange(bIsForRemoteClient)))
	{
		return true;
	}
	
	if (FailureReason)
	{
		FailureReason->AddTag(URGameplayTags::TAG_Ability_Fail_InvalidTarget);
	}
	return false;
}

bool UURGameplayAbility_ActiveAbility::CheckConsumeTag(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ensure(ActorInfo))
	{
		return true;
	}

	if (!ConsumeTags.IsEmpty())
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			if (!AbilitySystemComponent->HasAllMatchingGameplayTags(ConsumeTags))
			{
				if (OptionalRelevantTags)
				{
					const FGameplayTag& FailTag = URGameplayTags::TAG_Ability_Fail_Condition;
					if (FailTag.IsValid())
					{
						OptionalRelevantTags->AddTag(FailTag);
					}

					// Let the caller know which tags were blocking
					OptionalRelevantTags->AppendMatchingTags(AbilitySystemComponent->GetOwnedGameplayTags(),
					                                         ConsumeTags);
				}

				return false;
			}
		}
	}
	
	return true;
}

void UURGameplayAbility_ActiveAbility::ApplyConsumeTags(
	FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo& ActivationInfo)
{
	if (ConsumeTags.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			ASC->RemoveActiveEffectsWithGrantedTags(ConsumeTags);
		}
	}
}

void UURGameplayAbility_ActiveAbility::GrantInCombatGameplayEffect(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!InCombatGE)
	{
		return;
	}

	const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(
		Handle,
		ActorInfo,
		ActivationInfo,
		InCombatGE,
		GetAbilityLevel(Handle, ActorInfo));

	if (Spec.IsValid())
	{
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
	}
}

void UURGameplayAbility_ActiveAbility::GrantFollowupTag(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (FollowupTagGrantDuration <= SMALL_NUMBER ||
		FollowupTags.IsEmpty())
	{
		return;
	}

	if (const UGameplayEffect* ChainAbilityGE = UGameplayEffect_GrantTag::StaticClass()->GetDefaultObject<UGameplayEffect>())
	{
		// 쿨다운 시간 및 태그 적용
		const FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(
			Handle,
			ActorInfo,
			ActivationInfo,
			ChainAbilityGE->GetClass(),
			GetAbilityLevel(Handle, ActorInfo));

		if (Spec.IsValid())
		{
			// 태그 설정
			Spec.Data->DynamicGrantedTags.AppendTags(FollowupTags);
			// 시간 설정
			Spec.Data->SetSetByCallerMagnitude(URGameplayTags::SetByCaller_GrantTag, FollowupTagGrantDuration);
			ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
		}
	}
}

void UURGameplayAbility_ActiveAbility::ActionAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	FGameplayTagContainer FailureTags;
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo, &FailureTags))
	{
		EndAbilityAsFailed(FailureTags);
		return;
	}

	if (IsForRemoteClient())
	{
		// 서버에선 지연시간 보정을 위해 클라이언트의 Action 시작 시간 수신 대기
		UAbilityTask_SimpleWaitTargetData* SimpleWaitTargetData =
			UAbilityTask_SimpleWaitTargetData::SimpleWaitTargetData(this, URGameplayTags::TargetData_Time);
		SimpleWaitTargetData->ValidData.AddUniqueDynamic(this, &ThisClass::ValidateActionTimeTargetData);
		SimpleWaitTargetData->TimeOut.AddUniqueDynamic(this, &ThisClass::HandleActionTimeOut);
		SimpleWaitTargetData->ReadyForActivation();
		return;
	}

	const double ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	ClientDoActionTime = ServerTime;

	if (IsPredictingClient())
	{
		// 예측 클라이언트의 Action 시간 전송
		FGameplayAbilityTargetData_Time* TimeTargetData = new FGameplayAbilityTargetData_Time(ServerTime);
		const FGameplayAbilityTargetDataHandle TargetDataHandle(TimeTargetData);

		GetAbilitySystemComponentFromActorInfo()->CallServerSetReplicatedTargetData(
			GetCurrentAbilitySpecHandle(),
			GetCurrentActivationInfo().GetActivationPredictionKey(),
			TargetDataHandle,
			URGameplayTags::TargetData_Time,
			ActivationInfo.GetActivationPredictionKey());
	}

	PlayActionTimeline(Handle, ActorInfo, ActivationInfo);
}

void UURGameplayAbility_ActiveAbility::ValidateActionTimeTargetData(const FGameplayAbilityTargetDataHandle& Data)
{
	check(IsForRemoteClient()); // 서버에서만 실행되어야 함

	const FGameplayAbilityTargetDataHandle ReceivedData = Data;
	const FGameplayAbilityTargetData* TargetData = ReceivedData.Get(0);

	if (!TargetData)
	{
		// TargetData가 유효하지 않음
		K2_EndAbilityAsCancelled();
		return;
	}

	if (TargetData->GetScriptStruct() != FGameplayAbilityTargetData_Time::StaticStruct())
	{
		// TargetData가 유효하지 않음
		K2_EndAbilityAsCancelled();
		return;
	}

	const FGameplayAbilityTargetData_Time* TimeTargetData = static_cast<const FGameplayAbilityTargetData_Time*>(TargetData);

	/**
	 * Note: 클라이언트의 GameState->GetServerWorldTimeSeconds()도 서버에서 복제되기 때문에 단방향 Latency만큼 늦음
	 * 즉 서버의 현재 시간 - 클라이언트가 보낸 시간은 RTT임
	 * 보정을 위해 Latency에 0.5를 곱해야하나?
	 */

	const double ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	const double ClientTime = TimeTargetData->Time;
	double Latency = ServerTime - ClientTime;

	const UNetConnection* NetConnection = nullptr;
	if (const AActor* OwnerActor = GetOwningActorFromActorInfo())
	{
		NetConnection = OwnerActor->GetNetConnection();
	}
	if (!NetConnection)
	{
		if (const AActor* AvatarActor = GetAvatarActorFromActorInfo())
		{
			NetConnection = AvatarActor->GetNetConnection();
		}
	}

	if (NetConnection && NetConnection->AvgLag > SMALL_NUMBER)
	{
		// 클라이언트가 보낸 시간을 전적으로 신뢰하지 않고 평균 RTT와 비교하여 사용한다
		UR_LOG(LogUR, Log, TEXT("Validate Action Time AvgLag %f | Latency %f (%f | %f)"), NetConnection->AvgLag, Latency, TimeTargetData->Time, ServerTime);
		Latency = FMath::Clamp(FMath::Min(NetConnection->AvgLag, Latency), 0.f, MaxLatencyCompensation);
	}
	else
	{
		// Note: 원격 클라이언트인데 연결 정보를 얻지 못했다면 클라이언트의 타임 스탬프를 신뢰하지 않는다?
		// => 연결 정보를 얻지 못 할 경우가 많지 않고 PVE라 그냥 신뢰함.
		Latency = FMath::Clamp(Latency, 0.0f, MaxLatencyCompensation);
	}

	ClientDoActionTime = ServerTime - Latency;

	PlayActionTimeline(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo());
}

void UURGameplayAbility_ActiveAbility::HandleActionTimeOut()
{
	check(IsForRemoteClient()); // 서버에서만 실행되어야 함

	EndAbilityAsFailed(URGameplayTags::TAG_Ability_Fail_Networking);
}

void UURGameplayAbility_ActiveAbility::PlayActionTimeline(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	// 활성화 시간 타이머 설정
	if (ActiveDuration <= SMALL_NUMBER)
	{
		bIsActiveDurationOver = true;
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(ActiveDurationTimerHandle, this, &ThisClass::HandleActiveDurationOver, ActiveDuration, false);
	}
	
	AActor* AbilityTargetActor = AbilityTargetPtr.Get();
	if (AbilityTargetActor && 
		(TargetingPolicy == EAbilityTargetingPolicy::Targeting || TargetingPolicy == EAbilityTargetingPolicy::NonTargetingWithTarget))
	{
		// CombatComponent의 Target 설정
		if (UURCombatComponent* CombatComponent = ActorInfo->AvatarActor->FindComponentByClass<UURCombatComponent>())
		{
			CombatComponent->SetCombatTargetActor(AbilityTargetActor);
		}
		
		// TargetingComponent의 MainTarget이 비어있으면 설정
		if (IsLocallyControlled())
		{
			if (AURGamePlayerController* GamePC = Cast<AURGamePlayerController>(ActorInfo->PlayerController))
			{
				if (UURTargetingComponent* TargetingComponent = GamePC->GetTargetingComponent())
				{
					if (TargetingComponent->GetMainTargetActor() == nullptr)
					{
						TargetingComponent->SetMainTarget(AbilityTargetActor);
					}
				}
			}
		}
		
		// 타겟 바라보기
		UAbilityTask_LookTarget* LookTarget = UAbilityTask_LookTarget::LookTarget(this, AbilityTargetActor);
		LookTarget->ReadyForActivation();
	}
	else if (TargetingPolicy == EAbilityTargetingPolicy::Range)
	{
		FVector LookAtLocation = TargetLocation;
		if (UseRange <= SMALL_NUMBER)
		{
			// 본인 위치에서 사용하는 Range타입의 경우 카메라 정면 바라보도록 설정
			if (const APawn* Pawn = Cast<APawn>(ActorInfo->AvatarActor.Get()))
			{
				const FRotator MovementRotation(0.0f, Pawn->GetControlRotation().Yaw, 0.0f);

				LookAtLocation += MovementRotation.Vector() * 1000.f;
			}
		}
		
		UAbilityTask_LookTarget* LookTarget = UAbilityTask_LookTarget::LookTarget(this, LookAtLocation);
		LookTarget->ReadyForActivation();
	}

	const double ServerTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	const float Latency = ServerTime - ClientDoActionTime;

	FURAbilityActionContext Context;
	Context.Location = TargetLocation;
	Context.TargetActorPtr = AbilityTargetActor;

	UAbilityTask_AbilityActionTimeline* AbilityActionTimeline =
		UAbilityTask_AbilityActionTimeline::AbilityActionTimeline(
			this,
			Actions,
			Context,
			Latency,
			BlockMovementRange,
			BlockRotationRange);
	AbilityActionTimeline->OnCompleted.AddUniqueDynamic(this, &ThisClass::HandleActionTimelineCompleted);
	AbilityActionTimeline->OnFailed.AddUniqueDynamic(this, &ThisClass::HandleActionTimelineFailed);
	AbilityActionTimeline->ReadyForActivation();
}

void UURGameplayAbility_ActiveAbility::HandleActiveDurationOver()
{
	bIsActiveDurationOver = true;
	if ((bEndAbilityWhenActiveDurationOver || bIsTimelineCompleted) &&
		GetCurrentActorInfo()->IsNetAuthority())
	{
		K2_EndAbility();
	}
}

void UURGameplayAbility_ActiveAbility::HandleActionTimelineCompleted()
{
	// 정상 완료
	bIsTimelineCompleted = true;
	if (bIsActiveDurationOver && GetCurrentActorInfo()->IsNetAuthority())
	{
		K2_EndAbility();
	}
}

void UURGameplayAbility_ActiveAbility::HandleActionTimelineFailed()
{
	if (GetCurrentActorInfo()->IsNetAuthority())
	{
		EndAbilityAsFailed();
	}
}

#undef LOCTEXT_NAMESPACE
