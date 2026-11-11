// Written by EXPx2


#include "AbilitySystem/AbilityTask/AbilityActionTask/AbilityActionTask_MeleeTrace.h"

#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "AbilitySystem/GameplayEffect/URHitGameplayEffectContext.h"
#include "BlueprintLibrary/URGameBlueprintLibrary.h"
#include "Collisions/URColliderShape.h"
#include "BlueprintLibrary/URCollisionBlueprintLibrary.h"
#include "Combat/AbilityIndicatorActor.h"
#include "BlueprintLibrary/URCombatBlueprintLibrary.h"
#include "GameFramework/GameStateBase.h"

UAbilityActionTask_MeleeTrace* UAbilityActionTask_MeleeTrace::MeleeTrace(
	UURGameplayAbility_ActiveAbility* OwningAbility,
	const FURAbilityAction_MeleeTrace& ActionData,
	AActor* TargetActor,
	const float ElapsedTime)
{
	UAbilityActionTask_MeleeTrace* Task = NewAbilityTask<UAbilityActionTask_MeleeTrace>(OwningAbility);
	Task->bTickingTask = true;
	Task->ActionData = ActionData;
	Task->AbilityTargetActor = TargetActor;
	Task->ElapsedTime = FMath::Max(ElapsedTime, 0.0f);
	Task->LastHitActors.Reserve(ActionData.MaxTargetCount);
	return Task;
}

void UAbilityActionTask_MeleeTrace::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	UpdateTask();
}

void UAbilityActionTask_MeleeTrace::OnDestroy(bool bInOwnerFinished)
{
	SetPhase(EURMeleeTraceTaskPhase::Done);
	
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityActionTask_MeleeTrace::Activate()
{
	Super::Activate();
	
	ActivationWorldSeconds = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() - ElapsedTime;
	CastingEndTime = ActivationWorldSeconds + ActionData.CastingDuration;
	NextActionTime = ActionData.bHasCasting ? CastingEndTime : ActivationWorldSeconds;
	
	SetPhase(EURMeleeTraceTaskPhase::Casting);
	
	// ElapsedTime만큼 지연되어 시작하는데, 그 사이 진행된 Action을 처리하기 위해 업데이트
	UpdateTask();
}

bool UAbilityActionTask_MeleeTrace::FinishTask(const bool bIsSuccess)
{
	SetPhase(EURMeleeTraceTaskPhase::Done);
	
	return Super::FinishTask(bIsSuccess);
}

void UAbilityActionTask_MeleeTrace::UpdateTask()
{
	const double CurrentTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	
	AActor* OwnerActor = GetOwnerActor();
	AActor* AvatarActor = GetAvatarActor();
	
	if (!OwnerActor || !AvatarActor)
	{
		FinishTask(false);
		return;
	}
	
	if (CurrentPhase == EURMeleeTraceTaskPhase::Casting)
	{
		if (OwnerActor->HasAuthority())
		{
			// 인디케이터 업데이트는 서버에서 진행해 복제된다.
			if (AAbilityIndicatorActor* IndicatorActor = Indicator.Get())
			{
				FVector Location = FVector::ZeroVector;
				FVector Direction;
				GetLocationAndDirection(Location, Direction);
				IndicatorActor->SetActorLocation(Location);
				IndicatorActor->SetActorRotation(Direction.Rotation());
			}
		}
	
		if (CurrentTime >= CastingEndTime)
		{
			// 캐스팅 종료
			SetPhase(EURMeleeTraceTaskPhase::Action);
		}
		else
		{
			// 캐스팅 중
			return;
		}

	}

	// Note: 프레임 및 네트워크 지연으로 한 프레임에 여러번 가능하다.
	// 월드 재구성 등의 처리는 하지 않는다.
	while (CurrentPhase == EURMeleeTraceTaskPhase::Action &&
		CurrentTime >= NextActionTime)
	{
		Trace();
	}
}

void UAbilityActionTask_MeleeTrace::Trace()
{
	AActor* OwnerActor = GetOwnerActor();
	AActor* AvatarActor = GetAvatarActor();
	UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability);
	
	if (!OwnerActor || !AvatarActor || !ActiveAbility)
	{
		FinishTask(false);
		return;
	}
	
	if (OwnerActor->HasAuthority())
	{
		TArray<AActor*> TargetableActors;
		
		FVector Location = FVector::ZeroVector;
		FVector Direction;
		GetLocationAndDirection(Location, Direction);

		UURCollisionBlueprintLibrary::GetDamageableActors(
			this,
			Location,
			Direction,
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
			/*
			 * Note: 범위 내 대상 수가 MaxTargetCount보다 많을 경우 우선 순위를 통해 최대 대상 수 만큼만 지정한다.
			 * 1. 어빌리티 타겟: 스킬 사용 시 지정된 액터로 항상 최우선 순위로 삼는다.
			 * 2. 이전 히트에서 처리한 대상: 다회 공격할 때 이전 회차에서 공격한 대상을 대상으로 유지한다.
			 * 3. 그 외
			 */
			TArray<AActor*> HitActors;
			TArray<AActor*> PriorTargets = URUtilsLibrary::ToRawArray<AActor>(LastHitActors);
			
			UURCombatBlueprintLibrary::ResolveTargets(
				TargetableActors, 
				ActionData.MaxTargetCount,
				AbilityTargetActor.Get(),
				PriorTargets,
				HitActors);
			
			// 타겟 설정 완료
			Hit(HitActors);
		}
	}
	
	TraceCount++;
	
	if (TraceCount >= ActionData.HitCount)
	{
		FinishTask(true);
		return;
	}
	
	NextActionTime += ActionData.HitInterval;
}

void UAbilityActionTask_MeleeTrace::Hit(TArray<AActor*> Targets)
{
	LastHitActors.Empty(ActionData.MaxTargetCount);
	
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
		
		if (UURCombatBlueprintLibrary::Hit(ASC, Actor, ActionData.HitEffects, EffectContext))
		{
			LastHitActors.Add(Actor);
		}

		SetCount++;
		if (SetCount >= ActionData.MaxTargetCount)
		{
			// 정상 경로에선 Targets로 제한된 수의 액터가 넘어오긴 함.
			return;
		}
	}
}

void UAbilityActionTask_MeleeTrace::SetPhase(EURMeleeTraceTaskPhase NewPhase)
{
	if (CurrentPhase >= NewPhase)
	{
		// 이전 단계로 이동 불가
		return;
	}
	
	CurrentPhase = NewPhase;
	
	if (NewPhase == EURMeleeTraceTaskPhase::Casting)
	{
		if (!ActionData.bHasCasting || ActionData.CastingDuration <= SMALL_NUMBER)
		{
			// 캐스팅 없는 스킬
			SetPhase(EURMeleeTraceTaskPhase::Action);
			return;
		}
		
		if (ActionData.bShowIndicator && 
			GetOwnerActor() && 
			GetOwnerActor()->HasAuthority())
		{
			// 인디케이터 생성
			FVector Location = FVector::ZeroVector;
			FVector Direction;
			GetLocationAndDirection(Location, Direction);
			const FTransform Transform(Direction.Rotation(), Location);
			Indicator = AAbilityIndicatorActor::Spawn(
				GetWorld(),
				Transform,
				ActionData.Shape,
				ActivationWorldSeconds,
				ActionData.CastingDuration);
		}
		
		return;
	}
	
	// Casting 외의 페이즈
	// 인디케이터가 남아 있을 경우 제거
	if (GetOwnerActor() && GetOwnerActor()->HasAuthority())
	{
		if (AAbilityIndicatorActor* IndicatorActor = Indicator.Get())
		{
			IndicatorActor->Destroy();
		}
		Indicator.Reset();
	}
}

void UAbilityActionTask_MeleeTrace::GetLocationAndDirection(FVector& Location, FVector& Direction) const
{
	if (AActor* AvatarActor = GetAvatarActor())
	{
		Direction = AvatarActor->GetActorForwardVector().GetSafeNormal2D();
		Direction = Direction.RotateAngleAxis(ActionData.DirectionOffset, FVector::UpVector);
		
		Location = UURGameBlueprintLibrary::GetFeetLocation(AvatarActor);
		// Location은 바닥 위치이므로 Shape의 HalfHeight만큼 올려 Shape의 바닥을 맞춘다. 
		Location.Z += ActionData.Shape.Get<FURColliderShape>().HalfHeight;
		Location += Direction.Rotation().RotateVector(ActionData.LocationOffset);
	}
}
