// Written by EXPx2


#include "AbilitySystem/GameplayEffect/Calculation/URDamageExecutionCalculation.h"

#include "URGame.h"
#include "AbilitySystem/AttributeSet/URCombatSet.h"
#include "AbilitySystem/AttributeSet/URHealthSet.h"
#include "AbilitySystem/GameplayEffect/URHitGameplayEffectContext.h"
#include "GameplayTags/URGameplayTags.h"

namespace
{
	struct FDamageCaptures
	{
		const FGameplayEffectAttributeCaptureDefinition AttackPowerDef{UURCombatSet::GetAttackPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, true};
		const FGameplayEffectAttributeCaptureDefinition AdditionalAttackPowerDef{UURCombatSet::GetAdditionalAttackPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, true};
		const FGameplayEffectAttributeCaptureDefinition CriticalRateDef{UURCombatSet::GetCriticalRateAttribute(), EGameplayEffectAttributeCaptureSource::Source, true};
		const FGameplayEffectAttributeCaptureDefinition CriticalDamageDef{UURCombatSet::GetCriticalDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true};
		const FGameplayEffectAttributeCaptureDefinition DefenseDef{UURCombatSet::GetDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, true};
	};

	const FDamageCaptures& DamageCaptures()
	{
		static const FDamageCaptures Captures;
		return Captures;
	}
}

UURDamageExecutionCalculation::UURDamageExecutionCalculation()
{
	const FDamageCaptures& Captures = DamageCaptures();
	RelevantAttributesToCapture.Add(Captures.AttackPowerDef);
	RelevantAttributesToCapture.Add(Captures.AdditionalAttackPowerDef);
	RelevantAttributesToCapture.Add(Captures.CriticalRateDef);
	RelevantAttributesToCapture.Add(Captures.CriticalDamageDef);
	RelevantAttributesToCapture.Add(Captures.DefenseDef);
}

float UURDamageExecutionCalculation::CalculateAttackPower(
	const float AttackPower, 
	const float AdditionalAttackPower,
	const float CriticalRate, 
	const float CriticalDamage,
	bool& bIsCritical)
{
	const float Attack = FMath::Max(AttackPower, 0.f) +
		FMath::RandRange(0.f, FMath::Max(AdditionalAttackPower, 0.f));
	bIsCritical = FMath::FRand() < CriticalRate;
	return bIsCritical ? Attack * FMath::Max(CriticalDamage, 0.f) : Attack;
}

void UURDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const FDamageCaptures& Captures = DamageCaptures();
	
	float AttackPower = 0.0f;
	float AdditionalAttackPower = 0.0f;
	float CriticalRate = 0.0f;
	float CriticalDamage = 1.0f;
	float Defense = 0.0f;
	
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.AttackPowerDef, EvaluationParameters, AttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.AdditionalAttackPowerDef, EvaluationParameters, AdditionalAttackPower);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.CriticalRateDef, EvaluationParameters, CriticalRate);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.CriticalDamageDef, EvaluationParameters, CriticalDamage);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Captures.DefenseDef, EvaluationParameters, Defense);

	const float DamageMultiplier = Spec.GetSetByCallerMagnitude(URGameplayTags::SetByCaller_DamageMultiplier, false, 1.0f);
	
	bool bIsCritical = false;
	float Damage = CalculateAttackPower(AttackPower, AdditionalAttackPower, CriticalRate, CriticalDamage, bIsCritical);

	// 대미지 판정 추가 값 저장
	FGameplayEffectContext* Context = Spec.GetContext().Get();
	if (Context && Context->GetScriptStruct()->IsChildOf(FURHitGameplayEffectContext::StaticStruct()))
	{
		FURHitGameplayEffectContext* DamageContext = static_cast<FURHitGameplayEffectContext*>(Context);
		DamageContext->bIsCritical = bIsCritical;
	}

	Damage *= DamageMultiplier;
	
	// 피격자 방어력 적용
	Damage -= Defense;
	
	Damage = FMath::Max(Damage, 0.f);
	
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UURHealthSet::GetDamageTakenAttribute(), EGameplayModOp::Additive, Damage));
}
