// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_FindTargetAndTrace.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "AbilitySystem/GameplayEffect/URHitGameplayEffectContext.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Collisions/URColliderShape.h"
#include "BlueprintLibrary/URCollisionBlueprintLibrary.h"
#include "Combat/AbilityIndicatorActor.h"
#include "BlueprintLibrary/URCombatBlueprintLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTags/URGameplayTags.h"
#include "Particles/ParticleSystem.h"

UAbilityActionTask_FindTargetAndTrace* UAbilityActionTask_FindTargetAndTrace::FindTargetAndTrace(
	UURGameplayAbility_ActiveAbility* OwningAbility, 
	const FURAbilityAction_FindTargetAndTrace& ActionData,
	AActor* TargetActor, 
	float ElapsedTime)
{
	UAbilityActionTask_FindTargetAndTrace* Task = NewAbilityTask<UAbilityActionTask_FindTargetAndTrace>(OwningAbility);
	Task->bTickingTask = true;
	Task->ActionData = ActionData;
	Task->AbilityTargetActor = TargetActor;
	Task->ElapsedTime = FMath::Max(ElapsedTime, 0.0f);
	return Task;
}

void UAbilityActionTask_FindTargetAndTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	UpdateTask();
}

void UAbilityActionTask_FindTargetAndTrace::OnDestroy(bool bInOwnerFinished)
{
	SetPhase(EURTraceAtTargetTaskPhase::Done);
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityActionTask_FindTargetAndTrace::Activate()
{
	Super::Activate();
	
	TargetContainers.Reserve(ActionData.MaxFindingTargetCount);
	ActivationWorldSeconds = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() - ElapsedTime;
	CastingEndTime = ActivationWorldSeconds + ActionData.CastingDuration;
	TargetChasingEndTime = ActivationWorldSeconds + ActionData.ChasingDurationDuringCasting;
	NextActionTime = ActionData.bHasCasting ? CastingEndTime : ActivationWorldSeconds;
	
	SetPhase(EURTraceAtTargetTaskPhase::FindingTarget);
}

bool UAbilityActionTask_FindTargetAndTrace::FinishTask(const bool bIsSuccess)
{
	SetPhase(EURTraceAtTargetTaskPhase::Done);
	
	return Super::FinishTask(bIsSuccess);
}

void UAbilityActionTask_FindTargetAndTrace::FindTarget()
{
	AActor* AvatarActor = GetAvatarActor();
	UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability);
	
	if (!AvatarActor || !ActiveAbility)
	{
		FinishTask(false);
		return;
	}
	
	TArray<AActor*> TargetableActors;
	
	UURCollisionBlueprintLibrary::GetDamageableActors(
		GetWorld(),
		UURGameBlueprintLibrary::GetFeetLocation(AvatarActor),
		ActionData.FindingTargetRange,
		200.f,
		UURCombatBlueprintLibrary::GetRelativeTargetFaction(ActiveAbility->GetTargetFactions(), AvatarActor),
		AvatarActor,
		TargetableActors);
	
	int SetCount = 0;
	AActor* AbilityTarget = AbilityTargetActor.Get();
	
	// 어빌리티 타겟 우선 등록
	if (AbilityTarget && TargetableActors.Contains(AbilityTarget))
	{
		FURTraceAtTargetTargetContainer Container;
		Container.Target = AbilityTarget;
		Container.Location = UURGameBlueprintLibrary::GetFeetLocation(AbilityTarget);
		TargetContainers.Add(Container);
		SetCount++;
	}
	
	if (SetCount < ActionData.MaxFindingTargetCount)
	{
		for (AActor* Actor : TargetableActors)
		{
			// 어빌리티 타겟
			if (const bool bIsAbilityTaget = Actor == AbilityTarget)
			{
				// 위에서 이미 추가함
				continue;
			}
		
			FURTraceAtTargetTargetContainer Container;
			Container.Target = Actor;
			Container.Location = UURGameBlueprintLibrary::GetFeetLocation(Actor);
			TargetContainers.Add(Container);
			SetCount++;
		
			if (SetCount >= ActionData.MaxFindingTargetCount)
			{
				break;
			}
		}
	}
	
	if (SetCount < 1)
	{
		// 타겟 없을 경우
		FinishTask(true);
		return;
	}
	
	SetPhase(EURTraceAtTargetTaskPhase::Casting);
}

void UAbilityActionTask_FindTargetAndTrace::UpdateTask()
{
	const double CurrentTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	
	AActor* OwnerActor = GetOwnerActor();
	AActor* AvatarActor = GetAvatarActor();
	
	if (!OwnerActor || !AvatarActor)
	{
		FinishTask(false);
		return;
	}
	
	ClearInvalidTargetContainer();
	
	if (bIsFinished)
	{
		return;
	}
	
	if (CurrentPhase == EURTraceAtTargetTaskPhase::Casting)
	{
		if (CurrentTime < TargetChasingEndTime)
		{
			// 추적
			for (FURTraceAtTargetTargetContainer& TargetContainer : TargetContainers)
			{
				TargetContainer.Location = UURGameBlueprintLibrary::GetFeetLocation(TargetContainer.Target.Get());
				
				if (OwnerActor->HasAuthority())
				{
					// 인디케이터 업데이트는 서버에서 진행해 복제된다.
					if (AAbilityIndicatorActor* IndicatorActor = TargetContainer.Indicator.Get())
					{
						IndicatorActor->SetActorLocation(TargetContainer.Location);
					}
				}
			}
		}
	
		if (CurrentTime >= CastingEndTime)
		{
			// 캐스팅 종료
			SetPhase(EURTraceAtTargetTaskPhase::Action);
		}
		else
		{
			// 캐스팅 중
			return;
		}
	}
	
	Trace();
}

void UAbilityActionTask_FindTargetAndTrace::ClearInvalidTargetContainer()
{
	for (int i = TargetContainers.Num() - 1; i >= 0; i--)
	{
		FURTraceAtTargetTargetContainer& Container = TargetContainers[i];
		if (!Container.Target.IsValid())
		{
			// 타겟이 유효하지 않아지면 인디케이터와 함께 제거
			Container.Target.Reset();
			if (Container.Indicator.IsValid())
			{
				Container.Indicator->Destroy();
			}
			Container.Indicator.Reset();
			TargetContainers.RemoveAt(i);
		}
	}
	
	if (TargetContainers.Num() < 1)
	{
		// 모든 타겟이 유효하지 않아지면 태스크 종료 
		FinishTask(true);
	}
}

void UAbilityActionTask_FindTargetAndTrace::Trace()
{
	ClearInvalidTargetContainer();
	
	if (bIsFinished)
	{
		return;
	}
	
	AActor* OwnerActor = GetOwnerActor();
	AActor* AvatarActor = GetAvatarActor();
	UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability);
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get(); 
	
	if (!OwnerActor || !AvatarActor || !ActiveAbility || !ASC)
	{
		FinishTask(false);
		return;
	}
	
	if (OwnerActor->HasAuthority())
	{
		for (FURTraceAtTargetTargetContainer& Container : TargetContainers)
		{
			TArray<AActor*> TargetableActors;

			FVector Location = Container.Location;
			// Location은 바닥 위치이므로 Shape의 HalfHeight만큼 올려 Shape의 바닥을 맞춘다. 
			Location.Z += ActionData.Shape.Get<FURColliderShape>().HalfHeight;
			
			UURCollisionBlueprintLibrary::GetDamageableActors(
				this,
				Location,
				FVector::ForwardVector,
				ActionData.Shape,
				UURCombatBlueprintLibrary::GetRelativeTargetFaction(ActiveAbility->GetTargetFactions(), AvatarActor),
				AvatarActor,
				TargetableActors);
			
			if (TargetableActors.Num() <= ActionData.MaxTargetCount)
			{
				// 추가로 걸러낼 필요 없으면(0명이어도) 바로 Hit
				Hit(TargetableActors);
			}
			else
			{
				TArray<AActor*> HitActors;
				TArray<AActor*> PriorTargets;
				UURCombatBlueprintLibrary::ResolveTargets(
					TargetableActors,
					ActionData.MaxTargetCount,
					Container.Target.Get(),
					PriorTargets,
					HitActors);
				
				// 타겟 설정 완료
				Hit(HitActors);
			}
			
			// Cue
			if (UParticleSystem* Particle = ActionData.HitParticle.Get())
			{
				FGameplayCueParameters Params;
				Params.Location = Container.Location;
				Params.Instigator = OwnerActor;
				Params.EffectCauser = AvatarActor;
				Params.SourceObject = Particle;

				ASC->ExecuteGameplayCue(URGameplayTags::GameplayCue_Particle_Burst, Params);
			}
		}
	}
	
	FinishTask(true);
}

void UAbilityActionTask_FindTargetAndTrace::Hit(TArray<AActor*> Targets)
{
	UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo();
	UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability);
	if (!ensure(ASC) || !ActiveAbility)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = ActiveAbility->MakeHitEffectContext(Ability->GetCurrentAbilitySpecHandle(), Ability->GetCurrentActorInfo());
	if (EffectContext.Get() &&
		EffectContext.Get()->GetScriptStruct()->IsChildOf(FURHitGameplayEffectContext::StaticStruct()))
	{
		if (FURHitGameplayEffectContext* DamageContext = static_cast<FURHitGameplayEffectContext*>(EffectContext.Get()))
		{
			DamageContext->HitImpactCueTag = ActionData.HitImpactCueTag;
		}
	}
	
	int SetCount = 0;
	for (AActor* Actor : Targets)
	{
		if (!IsValid(Actor))
		{
			continue;
		}
		
		UURCombatBlueprintLibrary::Hit(ASC, Actor, ActionData.HitEffects, EffectContext);

		SetCount++;
		if (SetCount >= ActionData.MaxTargetCount)
		{
			// 정상 경로에선 Targets로 제한된 수의 액터가 넘어오긴 함.
			return;
		}
	}
}

void UAbilityActionTask_FindTargetAndTrace::SetPhase(EURTraceAtTargetTaskPhase NewPhase)
{
	if (CurrentPhase >= NewPhase)
	{
		// 이전 단계로 이동 불가
		return;
	}
	
	CurrentPhase = NewPhase;
	
	if (NewPhase == EURTraceAtTargetTaskPhase::FindingTarget)
	{
		FindTarget();
		return;
	}
	
	if (NewPhase == EURTraceAtTargetTaskPhase::Casting)
	{
		if (!ActionData.bHasCasting || ActionData.CastingDuration <= SMALL_NUMBER)
		{
			// 캐스팅 없는 스킬
			SetPhase(EURTraceAtTargetTaskPhase::Action);
			return;
		}
		
		if (ActionData.bShowIndicator && 
			GetOwnerActor() && 
			GetOwnerActor()->HasAuthority())
		{
			// 인디케이터 생성
			for (FURTraceAtTargetTargetContainer& Container : TargetContainers)
			{
				if (Container.Target.IsValid())
				{
					const FTransform Transform(FQuat::Identity, Container.Location);
					Container.Indicator = AAbilityIndicatorActor::Spawn(
						GetWorld(),
						Transform,
						ActionData.Shape,
						ActivationWorldSeconds,
						ActionData.CastingDuration);
				}
			}
		}
		
		return;
	}
	
	// 인디케이터가 남아 있을 경우 제거
	if (GetOwnerActor() && GetOwnerActor()->HasAuthority())
	{
		for (FURTraceAtTargetTargetContainer& Container : TargetContainers)
		{
			if (Container.Indicator.IsValid())
			{
				Container.Indicator->Destroy();
			}
			Container.Indicator.Reset();
		}
	}
}

void UAbilityActionTask_FindTargetAndTrace::GetLocationAndDirection(FVector& Location, FVector& Direction) const
{
}
