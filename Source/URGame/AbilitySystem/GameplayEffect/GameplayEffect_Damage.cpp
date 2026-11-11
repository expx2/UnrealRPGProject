// Written by EXPx2


#include "AbilitySystem/GameplayEffect/GameplayEffect_Damage.h"

#include "AbilitySystem/AttributeSet/URHealthSet.h"
#include "Calculation/URDamageExecutionCalculation.h"
#include "GameplayEffectComponents/AdditionalEffectsGameplayEffectComponent.h"

UGameplayEffect_Damage::UGameplayEffect_Damage()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Gameplay Effect - Additional Effects
	UAdditionalEffectsGameplayEffectComponent* AdditionalEffects = CreateDefaultSubobject<UAdditionalEffectsGameplayEffectComponent>(TEXT("AdditionalEffects"));
	GEComponents.Add(AdditionalEffects);
	static ConstructorHelpers::FClassFinder<UGameplayEffect> InCombatGERef(TEXT("/Game/_ProjectUR/GAS/GameplayEffects/BPGE_InCombat"));
	if (InCombatGERef.Succeeded())
	{
		FConditionalGameplayEffect ConditionalEffect;
		ConditionalEffect.EffectClass = InCombatGERef.Class;
		AdditionalEffects->OnApplicationGameplayEffects.Add(ConditionalEffect);
	}

	// Gameplay Effect - Executions
	FGameplayEffectExecutionDefinition Execution;
	Execution.CalculationClass = UURDamageExecutionCalculation::StaticClass();
	Executions.Add(Execution);

	// Gameplay Cues
	bRequireModifierSuccessToTriggerCues = true;
	FGameplayEffectCue Cue;
	Cue.MagnitudeAttribute = UURHealthSet::GetDamageTakenAttribute();
	Cue.GameplayCueTags.AddTag(URGameplayTags::GameplayCue_DamageTaken);
	GameplayCues.Add(Cue);
}
