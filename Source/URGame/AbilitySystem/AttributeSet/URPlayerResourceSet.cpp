// Written by EXPx2


#include "AbilitySystem/AttributeSet/URPlayerResourceSet.h"

#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UURPlayerResourceSet::UURPlayerResourceSet() :
	MaxMana(500.0f),
	Mana(500.0f),
	ManaRegen(10.0f)
{
}

void UURPlayerResourceSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UURPlayerResourceSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UURPlayerResourceSet, Mana, COND_None, REPNOTIFY_Always);
	// 스탯 표시를 위해 오너에게만 복제한다.
	DOREPLIFETIME_CONDITION_NOTIFY(UURPlayerResourceSet, ManaRegen, COND_OwnerOnly, REPNOTIFY_Always);
}

bool UURPlayerResourceSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	MaxManaBeforeAttributeChange = GetMaxMana();
	ManaBeforeAttributeChange = GetMana();

	return true;
}

void UURPlayerResourceSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (GetMaxMana() != MaxManaBeforeAttributeChange)
	{
		OnMaxManaChanged.Broadcast(Data.EvaluatedData.Magnitude, MaxManaBeforeAttributeChange, GetMaxMana());
	}
	if (GetMana() != ManaBeforeAttributeChange)
	{
		OnManaChanged.Broadcast(Data.EvaluatedData.Magnitude, ManaBeforeAttributeChange, GetMana());
	}
}

void UURPlayerResourceSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UURPlayerResourceSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UURPlayerResourceSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxManaAttribute() && GetMana() > NewValue)
	{
		// 최대 마나가 감소하면 현재 마나도 새 상한에 맞춘다.
		UURAbilitySystemComponent* URASC = GetOwningURAbilitySystemComponent();
		check(URASC);

		URASC->ApplyModToAttribute(GetManaAttribute(), EGameplayModOp::Override, NewValue);
	}
}

void UURPlayerResourceSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxMana());
	}
	else if (Attribute == GetMaxManaAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UURPlayerResourceSet::OnRep_MaxMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURPlayerResourceSet, MaxMana, OldValue);

	const float CurrentMaxMana = GetMaxMana();
	const float EstimatedMagnitude = CurrentMaxMana - OldValue.GetCurrentValue();
	OnMaxManaChanged.Broadcast(EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentMaxMana);
}

void UURPlayerResourceSet::OnRep_Mana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURPlayerResourceSet, Mana, OldValue);

	const float CurrentMana = GetMana();
	const float EstimatedMagnitude = CurrentMana - OldValue.GetCurrentValue();
	OnManaChanged.Broadcast(EstimatedMagnitude, OldValue.GetCurrentValue(), CurrentMana);
}

void UURPlayerResourceSet::OnRep_ManaRegen(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UURPlayerResourceSet, ManaRegen, OldValue);
}

