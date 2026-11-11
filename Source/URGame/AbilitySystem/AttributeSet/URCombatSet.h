// Written by EXPx2

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/AttributeSet/URAttributeSet.h"
#include "URCombatSet.generated.h"

/**
 * 
 */
UCLASS()
class URGAME_API UURCombatSet : public UURAttributeSet
{
	GENERATED_BODY()
	
public:
	UURCombatSet();
	
	ATTRIBUTE_ACCESSORS(UURCombatSet, AttackPower);
	ATTRIBUTE_ACCESSORS(UURCombatSet, AdditionalAttackPower);
	ATTRIBUTE_ACCESSORS(UURCombatSet, CriticalRate);
	ATTRIBUTE_ACCESSORS(UURCombatSet, CriticalDamage);
	ATTRIBUTE_ACCESSORS(UURCombatSet, Defense);
	ATTRIBUTE_ACCESSORS(UURCombatSet, AggroMultiplier);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_AdditionalAttackPower(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_CriticalRate(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue);
	
	// 공격력
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_AttackPower, Category="UR")
	FGameplayAttributeData AttackPower;
	
	// 추가 공격력
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_AdditionalAttackPower, Category="UR")
	FGameplayAttributeData AdditionalAttackPower;
	
	// 크리티컬 확률
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CriticalRate, Category="UR")
	FGameplayAttributeData CriticalRate;
	
	// 크리티컬 대미지
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_CriticalDamage, Category="UR")
	FGameplayAttributeData CriticalDamage;
	
	// 방어력
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Defense, Category="UR")
	FGameplayAttributeData Defense;
	
	// [NonRep] 어그로 수치 멀티플라이어
	UPROPERTY(BlueprintReadOnly, Category="UR")
	FGameplayAttributeData AggroMultiplier;
};
