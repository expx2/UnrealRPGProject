// Written by EXPx2


#include "AbilityActionTask_HitTarget.h"

#include "AbilitySystemComponent.h"
#include "BlueprintLibrary/URCombatBlueprintLibrary.h"
#include "AbilitySystem/GameplayAbility/URGameplayAbility_ActiveAbility.h"
#include "AbilitySystem/GameplayEffect/URHitGameplayEffectContext.h"
#include "GameFramework/GameStateBase.h"

UAbilityActionTask_HitTarget* UAbilityActionTask_HitTarget::HitTarget(
	UURGameplayAbility_ActiveAbility* OwningAbility,
	const FURAbilityAction_HitTarget& ActionData,
	AActor* TargetActor,
	const float ElapsedTime)
{
	UAbilityActionTask_HitTarget* Task = NewAbilityTask<UAbilityActionTask_HitTarget>(OwningAbility);
	Task->bTickingTask = true;
	Task->ActionData = ActionData;
	Task->AbilityTargetActor = TargetActor;
	Task->ElapsedTime = FMath::Max(ElapsedTime, 0.0f);
	return Task;
}

void UAbilityActionTask_HitTarget::TickTask(float DeltaTime)
{
	UpdateTask();
}

void UAbilityActionTask_HitTarget::Activate()
{
	Super::Activate();

	ActivationWorldSeconds = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() - ElapsedTime;
	NextActionTime = ActivationWorldSeconds;

	SetPhase(EURHitTargetTaskPhase::Action);

	UpdateTask();
}

bool UAbilityActionTask_HitTarget::FinishTask(const bool bIsSuccess)
{
	SetPhase(EURHitTargetTaskPhase::Done);

	return Super::FinishTask(bIsSuccess);
}

void UAbilityActionTask_HitTarget::UpdateTask()
{
	const double CurrentTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	
	// 프레임 지연으로 한 프레임에 여러번 가능
	while (CurrentPhase == EURHitTargetTaskPhase::Action && CurrentTime >= NextActionTime)
	{
		DoAction();
	}
}

void UAbilityActionTask_HitTarget::DoAction()
{
	if (GetOwnerActor()->HasAuthority())
	{
		UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability);
		if (!ActiveAbility)
		{
			FinishTask(false);
			return;
		}

		AActor* TargetActor = AbilityTargetActor.Get();
		if (ActiveAbility->IsValidTarget(GetOwnerActor(), TargetActor))
		{
			Hit(TargetActor);
		}
	}

	ActionCount++;

	if (ActionCount >= ActionData.HitCount)
	{
		FinishTask(true);
		return;
	}

	NextActionTime += ActionData.HitInterval;
}

void UAbilityActionTask_HitTarget::Hit(AActor* TargetActor)
{
	UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo();
	if (!ensure(ASC))
	{
		return;
	}
	
	const UURGameplayAbility_ActiveAbility* ActiveAbility = Cast<UURGameplayAbility_ActiveAbility>(Ability);
	FGameplayEffectContextHandle EffectContext = ActiveAbility
														   ? ActiveAbility->MakeHitEffectContext(Ability->GetCurrentAbilitySpecHandle(),Ability->GetCurrentActorInfo())
														   : Ability->MakeEffectContext(Ability->GetCurrentAbilitySpecHandle(),Ability->GetCurrentActorInfo());
	if (EffectContext.Get() &&
		EffectContext.Get()->GetScriptStruct()->IsChildOf(FURHitGameplayEffectContext::StaticStruct()))
	{
		if (FURHitGameplayEffectContext* DamageContext = static_cast<FURHitGameplayEffectContext*>(EffectContext.Get()))
		{
			DamageContext->HitImpactCueTag = ActionData.HitImpactTag;
		}
	}
	
	UURCombatBlueprintLibrary::Hit(ASC, TargetActor, ActionData.HitEffects, EffectContext);
}

void UAbilityActionTask_HitTarget::SetPhase(const EURHitTargetTaskPhase NewPhase)
{
	if (CurrentPhase >= NewPhase)
	{
		// 이전 단계로 이동 불가
		return;
	}

	CurrentPhase = NewPhase;
}
