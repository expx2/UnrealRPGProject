// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "URDamageExecutionCalculation.generated.h"

/**
 * (공격력 + 랜덤 추가 공격력) * 공격 계수 * 크리티컬 배율 - 방어력.
 */
UCLASS()
class URGAME_API UURDamageExecutionCalculation : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UURDamageExecutionCalculation();
	
	static float CalculateAttackPower(const float AttackPower, const float AdditionalAttackPower, const float CriticalRate, const float CriticalDamage, bool& bIsCritical);

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
