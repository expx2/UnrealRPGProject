// Written by EXPx2


#include "AbilitySystem/AttributeSet/URCombatSet.h"

#include "Net/UnrealNetwork.h"

UURCombatSet::UURCombatSet() :
	AttackPower(100.0f),
	AdditionalAttackPower(10.f),
	CriticalRate(0.1f),
	CriticalDamage(2.0f),
	Defense(10.f),
	AggroMultiplier(1.0f)
{
}

void UURCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 스탯 표시 위해 오너만 복제
	// 이 프로젝트에서 필요 없긴 함
	DOREPLIFETIME_CONDITION_NOTIFY(UURCombatSet, AttackPower, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UURCombatSet, AdditionalAttackPower, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UURCombatSet, CriticalRate, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UURCombatSet, CriticalDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UURCombatSet, Defense, COND_OwnerOnly, REPNOTIFY_Always);
}

void UURCombatSet::OnRep_AttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURCombatSet, AttackPower, OldValue);
}

void UURCombatSet::OnRep_AdditionalAttackPower(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURCombatSet, AdditionalAttackPower, OldValue);
}

void UURCombatSet::OnRep_CriticalRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURCombatSet, CriticalRate, OldValue);
}

void UURCombatSet::OnRep_CriticalDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURCombatSet, CriticalDamage, OldValue);
}

void UURCombatSet::OnRep_Defense(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURCombatSet, Defense, OldValue);
}
